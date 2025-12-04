/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "utils.h"
#include <string>
#include <vector>
#include <sstream>

namespace MericPlugin
{
std::vector<std::string>
split_string( const std::string& str, char delim )
{
    if ( str == "" )
    {
        return {};
    }
    size_t                   prev = 0;
    size_t                   pos  = 0;
    std::vector<std::string> split;
    while ( pos != str.npos )
    {
        pos = str.find( delim, prev );
        if ( pos != str.npos )
        {
            if ( pos - prev > 0 ) // two consecutive delimiters: do not add the empty string
            {
                split.emplace_back( str.substr( prev, pos - prev ) );
            }
        }
        else
        {
            if ( prev != str.size() ) // trailing delimiter: do not add the empty string
            {
                split.emplace_back( str.substr( prev ) );
            }
        }
        prev = pos + 1;
    }
    return split;
}

std::string
join_strings( const std::vector<std::string>& strings, std::string delim )
{
    if ( strings.empty() )
    {
        return "";
    }
    std::stringstream ss;
    auto              it = strings.begin();
    ss << *it;
    ++it;
    while ( it != strings.end() )
    {
        ss << delim << *it;
        ++it;
    }
    return ss.str();
}
}
