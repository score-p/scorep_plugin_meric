#include "energy_metric.h"

#include <scorep/plugin/plugin.hpp>

#include <chrono>
#include <string>


energy_metric::energy_metric( const std::string& name )
    : name( name ), ref( scorep::chrono::measurement_clock::now(), 0 )
{
}


bool
energy_metric::operator==( const energy_metric& other ) const
{
    return this->name == other.name;
}

TVPair
energy_metric::read() const
{
    const auto timestamp = scorep::chrono::measurement_clock::now();
    return TVPair( timestamp, 10. * ( timestamp - ref.first ).count() );
}
