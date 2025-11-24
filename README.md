<!--
SPDX-FileCopyrightText: NONE

SPDX-License-Identifier: CC0-1.0
-->
# Score-P metric plugin for reading energy values with MERIC extlib

## Prerequisites

- C++ compiler, supporting at least `c++14`
- [CMake](https://cmake.org/) > 3.12
- [Score-P](https://score-p.org/) > 6.0
- [MERIC](https://code.it4i.cz/energy-efficiency/meric-suite/meric):  with **[extlib](https://code.it4i.cz/energy-efficiency/meric-suite/meric/-/tree/external/src/extlib?ref_type=heads)**


## Quick start

### Build the plugin

```shell
mkdir -p _build && cd _build
cmake \
  -DCMAKE_INSTALL_PREFIX=${your_install_prefix} \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSCOREP_CONFIG=${path_to_scorep_config} \
  -DMeric_ROOT=${path_to_meric_installation} \
  ../
make
```

### Instrument an application with Score-P

Instrument your application as usual. See the [Score-P documentation](https://perftools.pages.jsc.fz-juelich.de/cicd/scorep/tags/latest/html/instrumentation.html)
The plugin is dynamically linked, nothing special needs to be done here.

### Run and collect measurements with the plugin

See also the documentation on [Score-P metric plugins](https://perftools.pages.jsc.fz-juelich.de/cicd/scorep/tags/latest/html/measurement.html#metric_plugins)

```shell
# Plugin is dynamically linked, provide the location of the plugin here
export LD_LIBRARY_PATH="${your_install_prefix}/lib:$LD_LIBRARY_PATH"
# Tell Score-P to use the 'meric_plugin'
export SCOREP_METRIC_PLUGINS=meric_plugin
# List the metrics that should be recorded
export SCOREP_METRIC_MERIC_PLUGIN=RAPL:package_0
# List the Meric energy domains that should be enabled
export SCOREP_METRIC_MERIC_PLUGIN_DOMAINS=RAPL,
# Set the sampling interval
export SCOREP_METRIC_MERIC_PLUGIN_INTERVAL_MS=20000
# This plugin is per-host, async, which only works with tracing
export SCOREP_ENABLE_PROFILING=0
export SCOREP_ENABLE_TRACING=1
# Optionally, the name of the measurement directory
export SCOREP_EXPERIMENT_DIRECTORY=scorep-measurement
# Run your application
./${your_application}
```

See the `test/` directory for an example.


## Contributing

### Developer tools

Add `-DMERIC_PLUGIN_DEVELOPER_MODE=1` to your `cmake` command to run some developer tools
as part of the build process. This includes

- (optional) [Uncrustify](https://github.com/uncrustify/uncrustify) to format CPP/C sources
- (optional) [Reuse](https://reuse.software/) to check compliance of project licenses
  with the reuse specification.


## License

License information is provided according to the [Reuse](https://reuse.software/)
specification. All files headers have short license identifiers and copyright notices.
Full license texts can be found in `LICENSES/`.
Most files are licensed under a BSD-3-Clause license.


## Acknowledgements

The examples provided in [scorep_plugin_cxx_wrapper](https://github.com/score-p/scorep_plugin_cxx_wrapper)
and the code from [scorep_plugin_x86_energy](https://github.com/score-p/scorep_plugin_x86_energy),
both developed at Technische Universität Dresden, were very helpful in the development of this plugin.

This code was developed as part of the [EUPEX project](https://eupex.eu/),
funded by EuroHPC and BMBF (Federal Ministry of Education and Research, Germany).
