/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <string>


std::vector<std::string>
split_string( const std::string& str,
              char               delim = ' ' );


template <typename K, typename V>
std::unordered_map<V, K>
map_inverse( const std::unordered_map<K, V>& map )
{
    std::unordered_map<V, K> inverse;
    inverse.reserve( map.size() );
    for ( const auto item : map )
    {
        auto emplace_result = inverse.emplace( item.second, item.first );
        if ( !emplace_result.second ) // could not insert the element
        {
            throw std::runtime_error( "Map is not invertible" );
        }
    }
    return inverse;
}


template <typename K, typename V>
std::vector<K>
map_keys( const std::unordered_map<K, V>& map )
{
    std::vector<K> keys;
    keys.reserve( map.size() );
    for ( const auto item : map )
    {
        keys.emplace_back( item.first );
    }
    return keys;
}
