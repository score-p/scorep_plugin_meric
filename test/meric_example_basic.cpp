/*
 * SPDX-FileCopyrightText: (c) 2024 IT4Innovations National Supercomputing Center, Ostrava, Czech Republic
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <meric_ext.h>

static float workarr[100];

void do_work(int nx, int ny)
{
    // Meaningless reference workload
    // Scales with the number of tests
    // Real applications would do some work in every test
    // iteration above, but in this benchmark it is
    // easier to measure if we extract the workload
    for(int w = 0;  w < 100; ++w)
    {
        workarr[w] = 0.;
    }
    for (int i = 0; i < nx; ++i)
    {
        for (int j = 0; j < ny; ++j)
        {
            const int idx = i*ny+j;
            workarr[idx%100] += (float)( idx%13 - 6 ) * (i%19);
        }
    }
}

int main() {
    ExtlibEnergy energy_domains = {0};
    EXTLIB_ENERGY_ENABLE_ALL_DOMAINS(energy_domains);

    extlib_init(&energy_domains, true);

    struct ExtlibEnergyTimeStamp *ts = extlib_read_energy_measurements(&energy_domains);

    do_work(10000, 100000);

    struct ExtlibEnergyTimeStamp *ts2 = extlib_read_energy_measurements(&energy_domains);

    struct ExtlibEnergyTimeStamp *result = extlib_calc_energy_consumption(ts, ts2);
    extlib_print_energy_timestamp(result, -1);

    extlib_free_energy_timestamp(ts);
    extlib_free_energy_timestamp(ts2);
    extlib_free_energy_timestamp(result);

    extlib_close(&energy_domains);
}
