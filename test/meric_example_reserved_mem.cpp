/*
 * SPDX-FileCopyrightText: (c) 2024 IT4Innovations National Supercomputing Center, Ostrava, Czech Republic
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <meric_ext.h>

int main()
{
    ExtlibEnergy energy_domains = {0};
    EXTLIB_ENERGY_ENABLE_ALL_DOMAINS(energy_domains);

    // Enable reserved memory for 10 measurements (automatically allocates 20 slots)
    // This is the page size, when more read is done, new pages are created with 20 slots and deallocated automatically.
    EXTLIB_ENERGY_USE_RESERVED_MEMORY(energy_domains, 10);

    extlib_init(&energy_domains, true);

    // All allocations now use pre-allocated memory pools
    struct ExtlibEnergyTimeStamp *ts1 = extlib_read_energy_measurements(&energy_domains);
    struct ExtlibEnergyTimeStamp *ts2 = extlib_read_energy_measurements(&energy_domains);

    struct ExtlibEnergyTimeStamp *result = extlib_calc_energy_consumption(ts1, ts2);

    // Cleanup - memory is returned to pool, not freed
    extlib_free_energy_timestamp(ts1);
    extlib_free_energy_timestamp(ts2);
    extlib_free_energy_timestamp(result);

    extlib_close(&energy_domains);
}
