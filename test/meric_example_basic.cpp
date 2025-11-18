/*
 * SPDX-FileCopyrightText: (c) 2024 IT4Innovations National Supercomputing Center, Ostrava, Czech Republic
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <meric_ext.h>

int main() {
    ExtlibEnergy energy_domains = {0};
    EXTLIB_ENERGY_ENABLE_ALL_DOMAINS(energy_domains);

    extlib_init(&energy_domains, true);

    struct ExtlibEnergyTimeStamp *ts = extlib_read_energy_measurements(&energy_domains);

    // some code of interest

    struct ExtlibEnergyTimeStamp *ts2 = extlib_read_energy_measurements(&energy_domains);

    struct ExtlibEnergyTimeStamp *result = extlib_calc_energy_consumption(ts, ts2);
    extlib_print_energy_timestamp(result, -1);

    extlib_free_energy_timestamp(ts);
    extlib_free_energy_timestamp(ts2);
    extlib_free_energy_timestamp(result);

    extlib_close(&energy_domains);
}
