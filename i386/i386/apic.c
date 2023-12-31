/* apic.c - APIC controller management for Mach.
   Copyright (C) 2020 Free Software Foundation, Inc.
   Written by Almudena Garcia Jurado-Centurion

   This file is part of GNU Mach.

   GNU Mach is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Mach is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA. */

#include <i386/apic.h>
#include <i386/cpu.h>
#include <i386at/idt.h>
#include <string.h>
#include <vm/vm_kern.h>
#include <kern/printf.h>
#include <kern/kalloc.h>


/*
 * This dummy structure is needed so that CPU_NUMBER can be called
 * before the lapic pointer is initialized to point to the real Local Apic.
 * It causes the apic_id to be faked as 0, which is the master processor.
 */
static ApicLocalUnit dummy_lapic = {0};
volatile ApicLocalUnit* lapic = &dummy_lapic;

/* This lookup table of [apic_id] -> kernel_id is initially populated with zeroes
 * so every lookup results in master processor until real kernel ids are populated.
 */
int cpu_id_lut[UINT8_MAX + 1] = {0};

ApicInfo apic_data;

/*
 * apic_data_init: initialize the apic_data structures to preliminary values.
 * Reserve memory to the lapic list dynamic vector.
 * Returns 0 if success, -1 if error.
 */
int
apic_data_init(void)
{
    apic_data.cpu_lapic_list = NULL;
    apic_data.ncpus = 0;
    apic_data.nioapics = 0;
    apic_data.nirqoverride = 0;

    /* Reserve the vector memory for the maximum number of processors. */
    apic_data.cpu_lapic_list = (uint16_t*) kalloc(NCPUS*sizeof(uint16_t));

    /* If the memory reserve fails, return -1 to advice about the error. */
    if (apic_data.cpu_lapic_list == NULL)
        return -1;

    return 0;
}

/*
 * apic_lapic_init: initialize lapic pointer to the memory common address.
 * Receives as input a pointer to the virtual memory address, previously mapped in a page.
 */
void
apic_lapic_init(ApicLocalUnit* lapic_ptr)
{
    lapic = lapic_ptr;
}

/*
 * apic_add_cpu: add a new lapic/cpu entry to the cpu_lapic list.
 * Receives as input the lapic's APIC ID.
 */
void
apic_add_cpu(uint16_t apic_id)
{
    apic_data.cpu_lapic_list[apic_data.ncpus] = apic_id;
    apic_data.ncpus++;
}

/*
 * apic_add_ioapic: add a new ioapic entry to the ioapic list.
 * Receives as input an ioapic_data structure, filled with the IOAPIC entry's data.
 */
void
apic_add_ioapic(IoApicData ioapic)
{
    apic_data.ioapic_list[apic_data.nioapics] = ioapic;
    apic_data.nioapics++;
}

/*
 * apic_add_irq_override: add a new IRQ to the irq_override list.
 * Receives as input an irq_override_data structure, filled with the IRQ entry's data.
 */
void
apic_add_irq_override(IrqOverrideData irq_over)
{
    apic_data.irq_override_list[apic_data.nirqoverride] = irq_over;
    apic_data.nirqoverride++;
}

IrqOverrideData *
acpi_get_irq_override(uint8_t pin)
{
    int i;

    for (i = 0; i < apic_data.nirqoverride; i++) {
        if (apic_data.irq_override_list[i].irq == pin) {
            return &apic_data.irq_override_list[i];
        }
    }
    return NULL;
}

/*
 * apic_get_cpu_apic_id: returns the apic_id of a cpu.
 * Receives as input the kernel ID of a CPU.
 */
int
apic_get_cpu_apic_id(int kernel_id)
{
    if (kernel_id >= NCPUS)
        return -1;

    return apic_data.cpu_lapic_list[kernel_id];
}


/*
 * apic_get_cpu_kernel_id: returns the kernel_id of a cpu.
 * Receives as input the APIC ID of a CPU.
 */
int
apic_get_cpu_kernel_id(uint16_t apic_id)
{
    return cpu_id_lut[apic_id];
}

/* apic_get_lapic: returns a reference to the common memory address for Local APIC. */
volatile ApicLocalUnit*
apic_get_lapic(void)
{
    return lapic;
}

/*
 * apic_get_ioapic: returns the IOAPIC identified by its kernel ID.
 * Receives as input the IOAPIC's Kernel ID.
 * Returns a ioapic_data structure pointer with the IOAPIC's data.
 */
struct IoApicData *
apic_get_ioapic(int kernel_id)
{
    if (kernel_id < MAX_IOAPICS)
        return &apic_data.ioapic_list[kernel_id];
    return NULL;
}

/* apic_get_numcpus: returns the current number of cpus. */
uint8_t
apic_get_numcpus(void)
{
    return apic_data.ncpus;
}

/* apic_get_num_ioapics: returns the current number of ioapics. */
uint8_t
apic_get_num_ioapics(void)
{
    return apic_data.nioapics;
}

/*
 * apic_get_current_cpu: returns the apic_id of current cpu.
 */
int
apic_get_current_cpu(void)
{
    unsigned int eax, ebx, ecx, edx;
    eax = 1;
    ecx = 0;
    cpuid(eax, ebx, ecx, edx);
    return (ebx >> 24);
}


/*
 * apic_refit_cpulist: adjust the size of cpu_lapic array to fit the real number of cpus
 * instead the maximum number.
 *
 * Returns 0 if success, -1 if error.
 */
int apic_refit_cpulist(void)
{
    uint16_t* old_list = apic_data.cpu_lapic_list;
    uint16_t* new_list = NULL;

    if (old_list == NULL)
        return -1;

    new_list = (uint16_t*) kalloc(apic_data.ncpus*sizeof(uint16_t));

    if (new_list == NULL)
        return -1;

    for (int i = 0; i < apic_data.ncpus; i++)
        new_list[i] = old_list[i];

    apic_data.cpu_lapic_list = new_list;
    kfree((vm_offset_t) old_list, NCPUS*sizeof(uint16_t));

    return 0;
}

/*
 * apic_generate_cpu_id_lut: Generate lookup table of cpu kernel ids from apic ids
 */
void apic_generate_cpu_id_lut(void)
{
    int i, apic_id;

    for (i = 0; i < apic_data.ncpus; i++) {
        apic_id = apic_get_cpu_apic_id(i);
        if (apic_id >= 0)
            cpu_id_lut[apic_id] = i;
        else
            printf("apic_get_cpu_apic_id(%d) failed...\n", i);
    }
}

/*
 * apic_print_info: shows the list of Local APIC and IOAPIC.
 * Shows each CPU and IOAPIC, with Its Kernel ID and APIC ID.
 */
void apic_print_info(void)
{
    int i;
    int ncpus, nioapics;

    ncpus = apic_get_numcpus();
    nioapics = apic_get_num_ioapics();

    uint16_t lapic_id;
    uint16_t ioapic_id;

    IoApicData *ioapic;

    printf("CPUS:\n");
    for (i = 0; i < ncpus; i++) {
        lapic_id = apic_get_cpu_apic_id(i);
        printf(" CPU %d - APIC ID %x - addr=0x%p\n", i, lapic_id, apic_get_lapic());
    }

    printf("IOAPICS:\n");
    for (i = 0; i < nioapics; i++) {
        ioapic = apic_get_ioapic(i);
        if (!ioapic) {
            printf("ERROR: invalid IOAPIC ID %x\n", i);
        } else {
            ioapic_id = ioapic->apic_id;
            printf(" IOAPIC %d - APIC ID %x - addr=0x%p\n", i, ioapic_id, ioapic->ioapic);
        }
    }
}

void apic_send_ipi(unsigned dest_shorthand, unsigned deliv_mode, unsigned dest_mode, unsigned level, unsigned trig_mode, unsigned vector, unsigned dest_id)
{
    IcrLReg icrl_values;
    IcrHReg icrh_values;

    icrl_values.destination_shorthand = dest_shorthand;
    icrl_values.delivery_mode = deliv_mode;
    icrl_values.destination_mode = dest_mode;
    icrl_values.level = level;
    icrl_values.trigger_mode = trig_mode;
    icrl_values.vector = vector;
    icrh_values.destination_field = dest_id;

    lapic->icr_high = icrh_values;
    lapic->icr_low = icrl_values;
}

void
lapic_enable(void)
{
    unsigned long flags;
    int apic_id;
    volatile uint32_t dummy;

    cpu_intr_save(&flags);

    apic_id = apic_get_current_cpu();

    dummy = lapic->dest_format.r;
    lapic->dest_format.r = 0xffffffff;		/* flat model */
    dummy = lapic->logical_dest.r;
    lapic->logical_dest.r = lapic->apic_id.r;	/* target self */
    dummy = lapic->lvt_lint0.r;
    lapic->lvt_lint0.r = dummy | LAPIC_DISABLE;
    dummy = lapic->lvt_lint1.r;
    lapic->lvt_lint1.r = dummy | LAPIC_DISABLE;
    dummy = lapic->lvt_performance_monitor.r;
    lapic->lvt_performance_monitor.r = dummy | LAPIC_DISABLE;
    if (apic_id != 0)
      {
        dummy = lapic->lvt_timer.r;
        lapic->lvt_timer.r = dummy | LAPIC_DISABLE;
      }
    dummy = lapic->task_pri.r;
    lapic->task_pri.r = 0;

    /* Enable LAPIC to send or recieve IPI/SIPIs */
    dummy = lapic->spurious_vector.r;
    lapic->spurious_vector.r = IOAPIC_SPURIOUS_BASE
			     | LAPIC_ENABLE_DIRECTED_EOI
			     | LAPIC_ENABLE;

    lapic->error_status.r = 0;

    cpu_intr_restore(flags);
}

void
lapic_eoi(void)
{
    lapic->eoi.r = 0;
}
