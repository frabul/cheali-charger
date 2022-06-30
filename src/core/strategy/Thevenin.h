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
#ifndef THEVENIN_H_
#define THEVENIN_H_

#include "AnalogInputs.h"

class Resistance {
  private:
    // R = iV/uI;
    // when discharging the resistance is negative (iV_ < 0)
    // in a Thevenin model this is mathematically equivalent
    // to a positive resistance and a current flowing in the opposite direction
    int16_t i_initial;
    uint16_t v_initial;
    int16_t i_last;
    uint16_t v_last;
    uint32_t resistance_value;
    bool discharge;

  public:
    const static int bit_shift = 14;
    Resistance();
    Resistance( uint16_t initial_value, bool discharge ) : resistance_value( initial_value ), discharge( discharge ) {
    }
    AnalogInputs::ValueType getReadableRth();
    void AddPoint( uint16_t i, int16_t v );
    /// Gets the reistance value as a fixed point number with bit_shift bits of precision
    uint32_t Get() const { return resistance_value; }
    static int32_t CalculateResistance( int32_t di, int32_t dv );
};

class Thevenin {
  public:
    AnalogInputs::ValueType Vth_;

  public:
    Resistance Rth;

    Thevenin() {
    }
    void calculateRthVth( AnalogInputs::ValueType v, AnalogInputs::ValueType i );
    void calculateVth( AnalogInputs::ValueType v, AnalogInputs::ValueType i );
    AnalogInputs::ValueType calculateI( AnalogInputs::ValueType Vc ) const;
    void init( AnalogInputs::ValueType Vth, AnalogInputs::ValueType Vmax, AnalogInputs::ValueType i, bool charge );
};

#endif /* THEVENIN_H_ */
