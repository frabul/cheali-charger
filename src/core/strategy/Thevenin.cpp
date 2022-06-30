/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stdlib.h>

#include "Thevenin.h"
#include "Utils.h"

int32_t Resistance::CalculateResistance( int32_t di, int32_t dv ) {
    return ( dv << Resistance::bit_shift ) / di;
}

void Resistance::AddPoint( uint16_t i, int16_t v ) {
    if ( i == 0 || v == 0 )
        return;

    if ( i_initial == 0 ) {
        // take first point
        i_initial = i;
        v_initial = v;
        v_last    = v;
        i_last    = i;
    } else {
        // accept only points coming from a current sweep ( current increasing)
        uint16_t min_increase = abs( i_last - i_initial ) >> 5;
        if ( i > i_last + min_increase ) {
            v_last = v;
            i_last = i;
            // calculate value
            int32_t dv = (int32_t)v_last - v_initial;
            int32_t di = (int32_t)i_last - i_initial;
            // update value if the dv change matches the one expected
            if ( ( discharge && dv < 0 ) || ( !discharge && dv > 0 ) ) {
                resistance_value = CalculateResistance( di, dv );
            }
        }
    }
}

AnalogInputs::ValueType Resistance::getReadableRth() {
    return ( resistance_value * ANALOG_VOLT( 1.0 ) ) >> bit_shift;
}

void Thevenin::init( AnalogInputs::ValueType Vth, AnalogInputs::ValueType Vmax, AnalogInputs::ValueType i, bool charge ) {
    AnalogInputs::ValueType Vfrom, Vto;
    if ( charge ) {
        // safety routine - important when one cell is overcharged
        Vfrom = min( Vth, Vmax );
        Vto   = max( Vth, Vmax );
    } else {
        Vfrom = max( Vth, Vmax );
        Vto   = min( Vth, Vmax );
    }
    Vth_        = Vfrom;
    int32_t res = Resistance::CalculateResistance( i, ( Vto - Vfrom ) );
    Rth         = Resistance( res, !charge );
}

AnalogInputs::ValueType Thevenin::calculateI( AnalogInputs::ValueType v ) const {

    int32_t v = ( (int32_t)v - Vth_ ) << Resistance::bit_shift; // shift before division
    int32_t i = v / Rth.Get();
    i         = ClipL( i, 0, UINT16_MAX );
    return i;
}

void Thevenin::calculateRthVth( AnalogInputs::ValueType v, AnalogInputs::ValueType i ) {
    Rth.AddPoint( v, i );
    calculateVth( v, i );
}

void Thevenin::calculateVth( AnalogInputs::ValueType v, AnalogInputs::ValueType i ) {
    int32_t VRth;
    // to be sure that product does not overflow
    // clip resistance value to 16 bit max ( it means that maximum is about 4 ohm)
    int32_t res = ClipInt32ToInt16( Rth.Get() );
    // fixed point number product
    VRth = ( i * res ) >> Resistance::bit_shift;
    // calculate vth
    if ( v < VRth )
        Vth_ = 0;
    else
        Vth_ = v - VRth;
}
