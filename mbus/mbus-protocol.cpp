//------------------------------------------------------------------------------
// Copyright (C) 2010-2011, Robert Johansson, Raditex AB
// All rights reserved.
//
// FreeSCADA 
// http://www.FreeSCADA.com
// freescada@freescada.com
//
//------------------------------------------------------------------------------

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mbus-protocol.h"

static int parse_debug = 0;

#define NITEMS(x) (sizeof(x)/sizeof(x[0]))

//------------------------------------------------------------------------------
// internal data
//------------------------------------------------------------------------------
static mbus_slave_data slave_data[MBUS_MAX_PRIMARY_SLAVES];

//------------------------------------------------------------------------------
// Return a pointer to the slave_data register. This register can be used for 
// storing current slave status.
//------------------------------------------------------------------------------
mbus_slave_data *
mbus_slave_data_get(size_t i)
{
    if (i < MBUS_MAX_PRIMARY_SLAVES)
    {
        return &slave_data[i];
    }
    
    return NULL;
}

//------------------------------------------------------------------------------
//
// M-Bus FRAME RELATED FUNCTIONS
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Allocate an M-bus frame data structure and initialize it according to which
// frame type is requested.
//------------------------------------------------------------------------------
mbus_frame *mbus_frame_new(int frame_type)
{
    mbus_frame *frame = NULL;
    
    if ((frame = malloc(sizeof(mbus_frame))) != NULL)
    {
        memset((void *)frame, 0, sizeof(mbus_frame));
        
        frame->type = frame_type;
        switch (frame->type)
        {
            case MBUS_FRAME_TYPE_ACK:
                frame->start1 = MBUS_FRAME_ACK_START;
                break;

            case MBUS_FRAME_TYPE_SHORT:
                frame->start1 = MBUS_FRAME_SHORT_START;
                frame->stop   = MBUS_FRAME_STOP;  
                break;

            case MBUS_FRAME_TYPE_CONTROL:
                frame->start1 = MBUS_FRAME_CONTROL_START;
                frame->start2 = MBUS_FRAME_CONTROL_START;
                frame->length1 = 3;
                frame->length2 = 3;
                frame->stop   = MBUS_FRAME_STOP;  
                break;

            case MBUS_FRAME_TYPE_LONG:
                frame->start1 = MBUS_FRAME_LONG_START;
                frame->start2 = MBUS_FRAME_LONG_START;
                frame->stop   = MBUS_FRAME_STOP;              
                break;
        }            
    }
    return frame;
}

//------------------------------------------------------------------------------
// Free the memory resources allocated for the M-Bus frame data structure.
//------------------------------------------------------------------------------
int
mbus_frame_free(mbus_frame *frame)
{
    if (frame)
    {
        free(frame);
        return 0;
    }
    return -1;
}

//------------------------------------------------------------------------------
// Caclulated the checksum of the M-Bus frame. Internal.
//------------------------------------------------------------------------------
u_char
calc_checksum(mbus_frame *frame)
{
    size_t i;
    u_char cksum;
    
    assert(frame != NULL);
    switch(frame->type)
    {
        case MBUS_FRAME_TYPE_SHORT:

            cksum = frame->control;
            cksum += frame->address;

            break;

        case MBUS_FRAME_TYPE_CONTROL:

            cksum = frame->control;
            cksum += frame->address;
            cksum += frame->control_information;

            break;

        case MBUS_FRAME_TYPE_LONG:

            cksum = frame->control;
            cksum += frame->address;
            cksum += frame->control_information;

            for (i = 0; i < frame->data_size; i++)
            {
                cksum += frame->data[i];
            }

            break;

        case MBUS_FRAME_TYPE_ACK:
        default:
            cksum = 0;
    }

    return cksum;
}

//------------------------------------------------------------------------------
// Caclulate the checksum of the M-Bus frame. The checksum algorithm is the
// arithmetic sum of the frame content, without using carry. Which content
// that is included in the checksum calculation depends on the frame type.
//------------------------------------------------------------------------------
int
mbus_frame_calc_checksum(mbus_frame *frame)
{
    if (frame)
    {
        switch (frame->type)
        {
            case MBUS_FRAME_TYPE_ACK:
            case MBUS_FRAME_TYPE_SHORT:
            case MBUS_FRAME_TYPE_CONTROL:
            case MBUS_FRAME_TYPE_LONG:
                frame->checksum = calc_checksum(frame);
            
                break;
        
            default:
                return -1;
        }        
    }
    
    return 0;
}

//
// Calculate the values of the lengths fields in the M-Bus frame. Internal.
//
u_char
calc_length(const mbus_frame *frame)
{
    assert(frame != NULL);
    switch(frame->type)
    {
        case MBUS_FRAME_TYPE_CONTROL:
            return 3;
        case MBUS_FRAME_TYPE_LONG:
            return frame->data_size + 3;
        default:
            return 0;
    }
}

//------------------------------------------------------------------------------
// Calculate the values of the lengths fields in the M-Bus frame.
//------------------------------------------------------------------------------
int
mbus_frame_calc_length(mbus_frame *frame)
{
    if (frame)
    {
        frame->length1 = frame->length2 = calc_length(frame);
    }

    return 0;
}

//------------------------------------------------------------------------------
// Return the M-Bus frame type
//------------------------------------------------------------------------------
int
mbus_frame_type(mbus_frame *frame)
{
    if (frame)
    {
        return frame->type;
    }
    return -1;
}

//------------------------------------------------------------------------------
// Verify that parsed frame is a valid M-bus frame. 
//
// Possible checks:
//
// 1) Start/stop bytes
// 2) length field and actual data size
// 3) checksum
//
//------------------------------------------------------------------------------
int
mbus_frame_verify(mbus_frame *frame)
{
    if (frame)
    {
        switch (frame->type)
        {
            case MBUS_FRAME_TYPE_ACK:
                return frame->start1 == MBUS_FRAME_ACK_START;

            case MBUS_FRAME_TYPE_SHORT:
                if(frame->start1 != MBUS_FRAME_SHORT_START)
                    return -1;

                break;

            case MBUS_FRAME_TYPE_CONTROL:
            case MBUS_FRAME_TYPE_LONG:
                if(frame->start1  != MBUS_FRAME_CONTROL_START ||
                   frame->start2  != MBUS_FRAME_CONTROL_START ||
                   frame->length1 != frame->length2 ||
                   frame->length1 != calc_length(frame))
                        return -1;

                break;

            default:
                return -1;
        }

        if(frame->stop != MBUS_FRAME_STOP ||
            frame->checksum != calc_checksum(frame))
                return -1;

        return 0;
    }

    return -1;
}


//------------------------------------------------------------------------------
//
// DATA ENCODING, DECODING, AND CONVERSION FUNCTIONS
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Encode BCD data
//------------------------------------------------------------------------------
int
mbus_data_bcd_encode(u_char *bcd_data, size_t bcd_data_size, int value)
{ 
    int val = 0, v0, v1, v2, x1, x2;
    size_t i;

    v2 = value;

    if (bcd_data)
    {
        for (i = 0; i < bcd_data_size; i++)
        {
            v0 = v2;
            v1 = (int)(v0 / 10.0);
            v2 = (int)(v1 / 10.0);

            x1 = v0 - v1 * 10;
            x2 = v1 - v2 * 10;

            bcd_data[bcd_data_size-1-i] = (x2 << 4) | x1;
        }

        return 0;    
    }
    
    return -1;
}

//------------------------------------------------------------------------------
// Decode BCD data
//------------------------------------------------------------------------------
int
mbus_data_bcd_decode(u_char *bcd_data, size_t bcd_data_size)
{       
    int val = 0;
    size_t i;
    
    if (bcd_data)
    {
        for (i = bcd_data_size; i > 0; i--)
        {
            val = (val * 10) + ((bcd_data[i-1]>>4) & 0xF);
            val = (val * 10) + ( bcd_data[i-1]     & 0xF);
        }

        return val;    
    }
    
    return -1;
}

//------------------------------------------------------------------------------
// Decode INTEGER data
//------------------------------------------------------------------------------
int
mbus_data_int_decode(u_char *int_data, size_t int_data_size)
{
    int val = 0;
    size_t i;
    
    if (int_data)
    {
        for (i = int_data_size; i > 0; i--)
        {
            val = (val << 8) + int_data[i-1];
        }

        return val;    
    }

    return -1;
}

long
mbus_data_long_decode(u_char *int_data, size_t int_data_size)
{
    long val = 0;
    size_t i;
    
    if (int_data)
    {
        for (i = int_data_size; i > 0; i--)
        {
            val = (val << 8) + int_data[i-1];
        }

        return val;    
    }

    return -1;
}

//------------------------------------------------------------------------------
// Encode INTEGER data (into 'int_data_size' bytes)
//------------------------------------------------------------------------------
int
mbus_data_int_encode(u_char *int_data, size_t int_data_size, int value)
{
    int val = 0, i;
    
    if (int_data)
    {
        for (i = 0; i < int_data_size; i++)
        {
            int_data[i] = (value>>(i*8)) & 0xFF;
        }

        return 0;
    }

    return -1;
}

void
mbus_data_str_decode(u_char *dst, const u_char *src, size_t len)
{
    size_t i;

    i = 0;
    dst[len] = '\0';
    while(len > 0) {
        dst[i++] = src[--len];
    }
}

//------------------------------------------------------------------------------
// Generate manufacturer code from 2-byte encoded data
//------------------------------------------------------------------------------
int
mbus_data_manufacturer_encode(u_char *m_data, u_char *m_code)
{
    int m_val;

    if (m_data == NULL || m_code == NULL)
        return -1;

    m_val = ((((int)m_code[0] - 64) & 0x001F) << 10) + 
            ((((int)m_code[1] - 64) & 0x001F) << 5) +
            ((((int)m_code[2] - 64) & 0x001F));

    mbus_data_int_encode(m_data, 2, m_val);
    
    return 0;
}

//------------------------------------------------------------------------------
// Generate manufacturer code from 2-byte encoded data
//------------------------------------------------------------------------------
const char *
mbus_decode_manufacturer(u_char byte1, u_char byte2)
{
    static char m_str[4];
    
    int m_id;
    
    m_str[0] = byte1;
    m_str[1] = byte2;
    
    m_id = mbus_data_int_decode(m_str, 2);
        
    m_str[0] = (char)(((m_id>>10) & 0x001F) + 64);
    m_str[1] = (char)(((m_id>>5)  & 0x001F) + 64);
    m_str[2] = (char)(((m_id)     & 0x001F) + 64);
    m_str[3] = 0;

    return m_str;
}

//------------------------------------------------------------------------------
//
// FIXED-LENGTH DATA RECORD FUNCTIONS
//
//------------------------------------------------------------------------------


//
//   Value         Field Medium/Unit              Medium
// hexadecimal Bit 16  Bit 15    Bit 8  Bit 7
//     0        0       0         0     0         Other
//     1        0       0         0     1         Oil
//     2        0       0         1     0         Electricity
//     3        0       0         1     1         Gas
//     4        0       1         0     0         Heat
//     5        0       1         0     1         Steam
//     6        0       1         1     0         Hot Water
//     7        0       1         1     1         Water
//     8        1       0         0     0         H.C.A.
//     9        1       0         0     1         Reserved
//     A        1       0         1     0         Gas Mode 2
//     B        1       0         1     1         Heat Mode 2
//     C        1       1         0     0         Hot Water Mode 2
//     D        1       1         0     1         Water Mode 2
//     E        1       1         1     0         H.C.A. Mode 2
//     F        1       1         1     1         Reserved
//
const char *
mbus_data_fixed_medium(mbus_data_fixed *data)
{
    static char buff[256];

    if (data)
    {
        switch ( (data->cnt1_type&0xC0)>>4 | (data->cnt2_type&0xC0)>>6 )
        {
            case 0x00:    
                snprintf(buff, sizeof(buff), "Other");
                break;    
            case 0x01:    
                snprintf(buff, sizeof(buff), "Oil");
                break;    
            case 0x02:    
                snprintf(buff, sizeof(buff), "Electricity");
                break;    
            case 0x03:    
                snprintf(buff, sizeof(buff), "Gas");
                break;    
            case 0x04:    
                snprintf(buff, sizeof(buff), "Heat");
                break;    
            case 0x05:    
                snprintf(buff, sizeof(buff), "Steam");
                break;    
            case 0x06:    
                snprintf(buff, sizeof(buff), "Hot Water");
                break;    
            case 0x07:    
                snprintf(buff, sizeof(buff), "Water");
                break;    
            case 0x08:    
                snprintf(buff, sizeof(buff), "H.C.A.");
                break;    
            case 0x09:    
                snprintf(buff, sizeof(buff), "Reserved");
                break;    
            case 0x0A:    
                snprintf(buff, sizeof(buff), "Gas Mode 2");
                break;
            case 0x0B:
                snprintf(buff, sizeof(buff), "Heat Mode 2");
                break;
            case 0x0C:
                snprintf(buff, sizeof(buff), "Hot Water Mode 2");
                break;
            case 0x0D:
                snprintf(buff, sizeof(buff), "Water Mode 2");
                break;
            case 0x0E:
                snprintf(buff, sizeof(buff), "H.C.A. Mode 2");
                break;    
            case 0x0F:
                snprintf(buff, sizeof(buff), "Reserved");
                break;    
            default:
                snprintf(buff, sizeof(buff), "unknown");
                break;                
        }
        
        return buff;
    }

    return NULL;
}


//------------------------------------------------------------------------------
//                        Hex code                            Hex code
//Unit                    share     Unit                      share
//              MSB..LSB                            MSB..LSB
//                       Byte 7/8                            Byte 7/8
// h,m,s         000000     00        MJ/h           100000     20
// D,M,Y         000001     01        MJ/h * 10      100001     21
//     Wh        000010     02        MJ/h * 100     100010     22
//     Wh * 10   000011     03        GJ/h           100011     23
//     Wh * 100  000100     04        GJ/h * 10      100100     24
//   kWh         000101     05        GJ/h * 100     100101     25
//  kWh   * 10   000110     06           ml          100110     26
//   kWh * 100   000111     07           ml * 10     100111     27
//   MWh         001000     08           ml * 100    101000     28
//   MWh * 10    001001     09            l          101001     29
//   MWh * 100   001010     0A            l * 10     101010     2A
//     kJ        001011     0B            l * 100    101011     2B
//     kJ * 10   001100     0C           m3          101100     2C
//     kJ * 100  001101     0D        m3 * 10        101101     2D
//     MJ        001110     0E        m3 * 100       101110     2E
//     MJ * 10   001111     0F        ml/h           101111     2F
//     MJ * 100  010000     10        ml/h * 10      110000     30
//     GJ        010001     11        ml/h * 100     110001     31
//     GJ * 10   010010     12         l/h           110010     32
//     GJ * 100  010011     13         l/h * 10      110011     33
//      W        010100     14         l/h * 100     110100     34                              
//      W * 10   010101     15       m3/h           110101     35                         
//      W * 100  010110     16     m3/h * 10       110110     36
//     kW        010111     17      m3/h * 100       110111     37
//     kW * 10   011000     18        °C* 10-3       111000     38
//     kW * 100  011001     19      units   for HCA  111001     39
//     MW        011010     1A    reserved           111010     3A
//     MW * 10   011011     1B    reserved           111011     3B
//     MW * 100  011100     1C    reserved           111100     3C
//  kJ/h         011101     1D    reserved           111101     3D
//  kJ/h * 10    011110     1E    same but historic  111110     3E
//  kJ/h * 100   011111     1F    without   units    111111     3F
//
//------------------------------------------------------------------------------
const char *
mbus_data_fixed_unit(int medium_unit_byte)
{
    static char buff[256];
    
    switch (medium_unit_byte & 0x3F)
    {
        case 0x00:    
            snprintf(buff, sizeof(buff), "h,m,s");
            break;    
        case 0x01:    
            snprintf(buff, sizeof(buff), "D,M,Y");
            break;    
            
        case 0x02:    
            snprintf(buff, sizeof(buff), "Wh");
            break;    
        case 0x03:    
            snprintf(buff, sizeof(buff), "10 Wh");
            break;    
        case 0x04:    
            snprintf(buff, sizeof(buff), "100 Wh");
            break;    
        case 0x05:    
            snprintf(buff, sizeof(buff), "kWh");
            break;    
        case 0x06:    
            snprintf(buff, sizeof(buff), "10 kWh");
            break;    
        case 0x07:    
            snprintf(buff, sizeof(buff), "100 kWh");
            break;    
        case 0x08:    
            snprintf(buff, sizeof(buff), "MWh");
            break;    
        case 0x09:    
            snprintf(buff, sizeof(buff), "10 MWh");
            break;    
        case 0x0A:    
            snprintf(buff, sizeof(buff), "100 MWh");
            break;    
            
        case 0x0B:    
            snprintf(buff, sizeof(buff), "kJ");
            break;    
        case 0x0C:    
            snprintf(buff, sizeof(buff), "10 kJ");
            break;    
        case 0x0E:    
            snprintf(buff, sizeof(buff), "100 kJ");
            break;    
        case 0x0D:    
            snprintf(buff, sizeof(buff), "MJ");
            break;    
        case 0x0F:    
            snprintf(buff, sizeof(buff), "10 MJ");
            break;    
        case 0x10:    
            snprintf(buff, sizeof(buff), "100 MJ");
            break;
        case 0x11:    
            snprintf(buff, sizeof(buff), "GJ");
            break;    
        case 0x12:    
            snprintf(buff, sizeof(buff), "10 GJ");
            break;    
        case 0x13:    
            snprintf(buff, sizeof(buff), "100 GJ");
            break;

        case 0x14:    
            snprintf(buff, sizeof(buff), "W");
            break;    
        case 0x15:    
            snprintf(buff, sizeof(buff), "10 W");
            break;    
        case 0x16:    
            snprintf(buff, sizeof(buff), "100 W");
            break;    
        case 0x17:    
            snprintf(buff, sizeof(buff), "kW");
            break;    
        case 0x18:    
            snprintf(buff, sizeof(buff), "10 kW");
            break;    
        case 0x19:    
            snprintf(buff, sizeof(buff), "100 kW");
            break;
        case 0x1A:    
            snprintf(buff, sizeof(buff), "MW");
            break;    
        case 0x1B:    
            snprintf(buff, sizeof(buff), "10 MW");
            break;    
        case 0x1C:    
            snprintf(buff, sizeof(buff), "100 MW");
            break;
            
        case 0x29:    
            snprintf(buff, sizeof(buff), "l");
            break;
        case 0x2A:    
            snprintf(buff, sizeof(buff), "10 l");
            break;
        case 0x2B:    
            snprintf(buff, sizeof(buff), "100 l");
            break;
        case 0x3E:    
            snprintf(buff, sizeof(buff), "same but historic");
            break;  
        default:
            snprintf(buff, sizeof(buff), "unknown");
            break;    
    }

    return buff;
}

//------------------------------------------------------------------------------
//
// VARIABLE-LENGTH DATA RECORD FUNCTIONS
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Medium                                                              Code bin    Code hex
// Other                                                              0000 0000        00
// Oil                                                                0000 0001        01
// Electricity                                                        0000 0010        02
// Gas                                                                0000 0011        03
// Heat (Volume measured at return temperature: outlet)               0000 0100        04
// Steam                                                              0000 0101        05
// Hot Water                                                          0000 0110        06
// Water                                                              0000 0111        07
// Heat Cost Allocator.                                               0000 1000        08
// Compressed Air                                                     0000 1001        09
// Cooling load meter (Volume measured at return temperature: outlet) 0000 1010        0A
// Cooling load meter (Volume measured at flow temperature: inlet) ♣  0000 1011        0B
// Heat (Volume measured at flow temperature: inlet)                  0000 1100        0C
// Heat / Cooling load meter ♣                                        0000 1101        OD
// Bus / System                                                       0000 1110        0E
// Unknown Medium                                                     0000 1111        0F
// Reserved                                                           .......... 10 to 15
// Cold Water                                                         0001 0110        16
// Dual Water                                                         0001 0111        17
// Pressure                                                           0001 1000        18
// A/D Converter                                                      0001 1001        19
// Reserved                                                           .......... 20 to FF
//------------------------------------------------------------------------------
const char *
mbus_data_variable_medium_lookup(u_char medium)
{
    static char buff[256];

    switch (medium)
    {
        case MBUS_VARIABLE_DATA_MEDIUM_OTHER:
            snprintf(buff, sizeof(buff), "Other");
            break;
                                          
        case MBUS_VARIABLE_DATA_MEDIUM_OIL:
            snprintf(buff, sizeof(buff), "Oil");
            break;
                  
        case MBUS_VARIABLE_DATA_MEDIUM_ELECTRICITY:
            snprintf(buff, sizeof(buff), "Electricity");
            break;
                                          
        case MBUS_VARIABLE_DATA_MEDIUM_GAS:
            snprintf(buff, sizeof(buff), "Gas");
            break;
                                          
        case MBUS_VARIABLE_DATA_MEDIUM_HEAT:
            snprintf(buff, sizeof(buff), "Heat");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_STEAM:
            snprintf(buff, sizeof(buff), "Steam");
            break;
                                          
        case MBUS_VARIABLE_DATA_MEDIUM_HOT_WATER:
            snprintf(buff, sizeof(buff), "Hot water");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_WATER:
            snprintf(buff, sizeof(buff), "Water");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_HEAT_COST:
            snprintf(buff, sizeof(buff), "Heat Cost Allocator");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_COMPR_AIR:
            snprintf(buff, sizeof(buff), "Compressed Air");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_COOL_OUT:
            snprintf(buff, sizeof(buff), "Cooling load meter: Outlet");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_COOL_IN:
            snprintf(buff, sizeof(buff), "Cooling load meter: Inlet");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_BUS:
            snprintf(buff, sizeof(buff), "Bus/System");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_COLD_WATER:
            snprintf(buff, sizeof(buff), "Cold water");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_DUAL_WATER:
            snprintf(buff, sizeof(buff), "Dual water");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_PRESSURE:
            snprintf(buff, sizeof(buff), "Pressure");
            break;

        case MBUS_VARIABLE_DATA_MEDIUM_ADC:
            snprintf(buff, sizeof(buff), "A/D Converter");
            break;

        case 0x0C:
            snprintf(buff, sizeof(buff), "Heat (Volume measured at flow temperature: inlet)");
            break;

        case 0x20: // - 0xFF
            snprintf(buff, sizeof(buff), "Reserved");
            break;

                                          
        // add more ...
        default:
            snprintf(buff, sizeof(buff), "Unknown medium (0x%.2x)", medium);
            break;            
    }

    return buff;       
}

//------------------------------------------------------------------------------
// Lookup the unit description from a VIF field in a data record
//------------------------------------------------------------------------------
const char *
mbus_unit_prefix(int exp)
{
    static char buff[256];
    
    switch (exp)
    {
        case 0:
            buff[0] = 0;
            break;

        case -3:
            snprintf(buff, sizeof(buff), "m");
            break;
       
        case -6:
            snprintf(buff, sizeof(buff), "my");
            break;

        case 1:
            snprintf(buff, sizeof(buff), "10 ");
            break;

        case 2:
            snprintf(buff, sizeof(buff), "100 ");
            break;

        case 3:
            snprintf(buff, sizeof(buff), "k");
            break;
            
        case 4:
            snprintf(buff, sizeof(buff), "10 k");
            break;            

        case 5:
            snprintf(buff, sizeof(buff), "100 k");
            break;            

        case 6:
            snprintf(buff, sizeof(buff), "M");
            break;
       
        case 9:
            snprintf(buff, sizeof(buff), "T");
            break;
           
        default:
            snprintf(buff, sizeof(buff), "10^%d ", exp);
    }
        
    return buff;
}

//------------------------------------------------------------------------------
// Look up the unit from a VIF field in the data record.
// 
// See section 8.4.3  Codes for Value Information Field (VIF) in the M-BUS spec
//------------------------------------------------------------------------------
const char *
mbus_vif_unit_lookup(u_char vif)
{
    static char buff[256];
    int n;

    switch (vif)
    {
        // E000 0nnn Energy 10(nnn-3) W
        case 0x00:
        case 0x00+1:
        case 0x00+2:
        case 0x00+3:
        case 0x00+4:
        case 0x00+5:
        case 0x00+6:
        case 0x00+7:
            n = (vif & 0x07) - 3;
            snprintf(buff, sizeof(buff), "Energy (%sWh)", mbus_unit_prefix(n)); 
            break;

        // 0000 1nnn          Energy       10(nnn)J     (0.001kJ to 10000kJ)
        case 0x08:
        case 0x08+1:
        case 0x08+2:
        case 0x08+3:
        case 0x08+4:
        case 0x08+5:
        case 0x08+6:
        case 0x08+7:
            
            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Energy (%sJ)", mbus_unit_prefix(n));
  
            break;        

        // E001 1nnn Mass 10(nnn-3) kg 0.001kg to 10000kg
        case 0x18:
        case 0x18+1:
        case 0x18+2:
        case 0x18+3:
        case 0x18+4:
        case 0x18+5:
        case 0x18+6:
        case 0x18+7:
            
            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Mass (%skg)", mbus_unit_prefix(n-3));
  
            break;        

        // E010 1nnn Power 10(nnn-3) W 0.001W to 10000W
        case 0x28:
        case 0x28+1:
        case 0x28+2:
        case 0x28+3:
        case 0x28+4:
        case 0x28+5:
        case 0x28+6:
        case 0x28+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Power (%sW)", mbus_unit_prefix(n-3));
            //snprintf(buff, sizeof(buff), "Power (10^%d W)", n-3);
  
            break;
            
        // E011 0nnn Power 10(nnn) J/h 0.001kJ/h to 10000kJ/h
        case 0x30:
        case 0x30+1:
        case 0x30+2:
        case 0x30+3:
        case 0x30+4:
        case 0x30+5:
        case 0x30+6:
        case 0x30+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Power (%sJ/h)", mbus_unit_prefix(n));
  
            break;       

        // E001 0nnn Volume 10(nnn-6) m3 0.001l to 10000l
        case 0x10:
        case 0x10+1:
        case 0x10+2:
        case 0x10+3:
        case 0x10+4:
        case 0x10+5:
        case 0x10+6:
        case 0x10+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Volume (%s m^3)", mbus_unit_prefix(n-6));
  
            break;
        
        // E011 1nnn Volume Flow 10(nnn-6) m3/h 0.001l/h to 10000l/
        case 0x38:
        case 0x38+1:
        case 0x38+2:
        case 0x38+3:
        case 0x38+4:
        case 0x38+5:
        case 0x38+6:
        case 0x38+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Volume flow (%s m^3/h)", mbus_unit_prefix(n-6));
  
            break;     

        // E100 0nnn Volume Flow ext. 10(nnn-7) m3/min 0.0001l/min to 1000l/min
        case 0x40:
        case 0x40+1:
        case 0x40+2:
        case 0x40+3:
        case 0x40+4:
        case 0x40+5:
        case 0x40+6:
        case 0x40+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Volume flow (%s m^3/min)", mbus_unit_prefix(n-7));
  
            break;  

        // E100 1nnn Volume Flow ext. 10(nnn-9) m3/s 0.001ml/s to 10000ml/
        case 0x48:
        case 0x48+1:
        case 0x48+2:
        case 0x48+3:
        case 0x48+4:
        case 0x48+5:
        case 0x48+6:
        case 0x48+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Volume flow (%s m^3/s)", mbus_unit_prefix(n-9));
  
            break;       

        // E101 0nnn Mass flow 10(nnn-3) kg/h 0.001kg/h to 10000kg/
        case 0x50:
        case 0x50+1:
        case 0x50+2:
        case 0x50+3:
        case 0x50+4:
        case 0x50+5:
        case 0x50+6:
        case 0x50+7:

            n = (vif & 0x07);
            snprintf(buff, sizeof(buff), "Mass flow (%s kg/h)", mbus_unit_prefix(n-3));
  
            break;       

        // E101 10nn Flow Temperature 10(nn-3) °C 0.001°C to 1°C
        case 0x58:
        case 0x58+1:
        case 0x58+2:
        case 0x58+3:

            n = (vif & 0x03);
            snprintf(buff, sizeof(buff), "Flow temperature (%sdeg C)", mbus_unit_prefix(n-3));
  
            break;

        // E101 11nn Return Temperature 10(nn-3) °C 0.001°C to 1°C
        case 0x5C:
        case 0x5C+1:
        case 0x5C+2:
        case 0x5C+3:

            n = (vif & 0x03);
            snprintf(buff, sizeof(buff), "Return temperature (%sdeg C)", mbus_unit_prefix(n-3));
  
            break;

        // E110 10nn Pressure 10(nn-3) bar 1mbar to 1000mbar
        case 0x68:
        case 0x68+1:
        case 0x68+2:
        case 0x68+3:

            n = (vif & 0x03);
            snprintf(buff, sizeof(buff), "Pressure (%s bar)", mbus_unit_prefix(n-3));
  
            break;

        // E010 00nn On Time
        // nn = 00 seconds
        // nn = 01 minutes
        // nn = 10   hours
        // nn = 11    days
        // E010 01nn Operating Time coded like OnTime
        case 0x20:
        case 0x20+1:
        case 0x20+2:
        case 0x20+3:
        case 0x24:
        case 0x24+1:
        case 0x24+2:
        case 0x24+3:
            {
                int offset;

                if (vif & 0x4)
                    offset = snprintf(buff, sizeof(buff), "Operating time ");
                else
                    offset = snprintf(buff, sizeof(buff), "On time ");
                
                switch (vif & 0x03)
                {
                    case 0x00:
                        snprintf(&buff[offset], sizeof(buff)-offset, "(seconds)");
                        break;
                    case 0x01:
                        snprintf(&buff[offset], sizeof(buff)-offset, "(minutes)");
                        break;
                    case 0x02:
                        snprintf(&buff[offset], sizeof(buff)-offset, "(hours)");
                        break;
                    case 0x03:
                        snprintf(&buff[offset], sizeof(buff)-offset, "(days)");
                        break;
                }
            }
            break;                     

        // E110 110n Time Point
        // n = 0        date
        // n = 1 time & date
        // data type G
        // data type F
        case 0x6C:
        case 0x6C+1:

            if (vif & 0x1)
                snprintf(buff, sizeof(buff), "Time Point (time & date)");
            else
                snprintf(buff, sizeof(buff), "Time Point (date)");
  
            break;
            
        // E110 00nn    Temperature Difference   10(nn-3)K   (mK to  K)
        case 0x60:
        case 0x60+1:
        case 0x60+2:
        case 0x60+3:

            n = (vif & 0x03);
            
            snprintf(buff, sizeof(buff), "Temperature Difference (%s deg C)", mbus_unit_prefix(n-3));
                        
            break;

        // E110 01nn External Temperature 10(nn-3) °C 0.001°C to 1°C
        case 0x64:
        case 0x64+1:
        case 0x64+2:
        case 0x64+3:

            n = (vif & 0x03);
            snprintf(buff, sizeof(buff), "External temperature (%s deg C)", mbus_unit_prefix(n-3));
  
            break;

        // E110 1110 Units for H.C.A. dimensionless
        case 0x6E:
            snprintf(buff, sizeof(buff), "Units for H.C.A.");
            break; 

        // E110 1111 Reserved
        case 0x6F:
            snprintf(buff, sizeof(buff), "Reserved");
            break;        

        // Fabrication No
        case 0x78:
            snprintf(buff, sizeof(buff), "Fabrication number");
            break;        

        // Manufacturer specific: 7Fh / FF  
        case 0x7F:
        case 0xFF:
            snprintf(buff, sizeof(buff), "Manufacturer specific");
            break;
                       
        default:
        
            snprintf(buff, sizeof(buff), "Unknown (VIF=0x%.2X)", vif);
            break;
    }
    

    return buff;
}

//------------------------------------------------------------------------------
// Lookup the unit from the VIB (VIF or VIFE)
//------------------------------------------------------------------------------
const char *
mbus_vib_unit_lookup(mbus_value_information_block *vib)
{   
    static char buff[256];
    int n;

    if (vib->vif == 0xFD) // first type of VIF extention: see table 8.4.4 
    {
        if (vib->nvife == 0)
        {
            snprintf(buff, sizeof(buff), "Missing VIF extension");
        }
        else if (vib->vife[0] == 0x10)
        {
            // VIFE = E001 0000 Customer location
            snprintf(buff, sizeof(buff), "Customer location");
        }
        else if (vib->vife[0] == 0x0C)
        {
            // E000 1100 Model / Version
            snprintf(buff, sizeof(buff), "Model / Version");
        }
        else if (vib->vife[0] == 0x11)
        {
            // VIFE = E001 0001 Customer
            snprintf(buff, sizeof(buff), "Customer");
        }
        else if (vib->vife[0] == 0x9)
        {
            // VIFE = E001 0110 Password
            snprintf(buff, sizeof(buff), "Password");
        }
        else if (vib->vife[0] == 0x0b)
        {
            // VIFE = E000 1011 Parameter set identification
            snprintf(buff, sizeof(buff), "Parameter set identification");
        }
        else if ((vib->vife[0] & 0xF0) == 0x70)
        {
            // VIFE = E111 nnn Reserved
            snprintf(buff, sizeof(buff), "Reserved VIF extension");
        }
        else
        {
            snprintf(buff, sizeof(buff), "Unrecongized VIF extension: 0x%.2x", vib->vife[0]);
        }
        return buff;
    }
    
    return mbus_vif_unit_lookup(vib->vif); // no extention, use VIF
}

//------------------------------------------------------------------------------
// Decode data and write to string
//
// Data format (for record->data data array)
// 
// Length in Bit   Code    Meaning           Code      Meaning
//      0          0000    No data           1000      Selection for Readout
//      8          0001     8 Bit Integer    1001      2 digit BCD
//     16          0010    16 Bit Integer    1010      4 digit BCD
//     24          0011    24 Bit Integer    1011      6 digit BCD
//     32          0100    32 Bit Integer    1100      8 digit BCD
//   32 / N        0101    32 Bit Real       1101      variable length
//     48          0110    48 Bit Integer    1110      12 digit BCD
//     64          0111    64 Bit Integer    1111      Special Functions
//
// The Code is stored in record->drh.dib.dif
//
// Return a string containing the data
//
// Source: MBDOC48.PDF
//
//------------------------------------------------------------------------------
const char *
mbus_data_record_decode(mbus_data_record *record)
{
    static char buff[256];

    if (record)
    {
        int val;
        long val2;
            
        switch (record->drh.dib.dif & 0x0F)
        {
            case 0x00: // no data
        
                buff[0] = 0;
                
                break; 

            case 0x01: // 1 byte integer (8 bit)
        
                val = mbus_data_int_decode(record->data, 1);
               
                snprintf(buff, sizeof(buff), "%d", val);

                break; 


            case 0x02: // 2 byte integer (16 bit)
        
                val = mbus_data_int_decode(record->data, 2);
        
                snprintf(buff, sizeof(buff), "%d", val);

                break; 

            case 0x03: // 3 byte integer (24 bit)

                val = mbus_data_int_decode(record->data, 3);
                        
                snprintf(buff, sizeof(buff), "%d", val);

                break; 
                
            case 0x04: // 4 byte integer (32 bit)

                val = mbus_data_int_decode(record->data, 4);
        
                snprintf(buff, sizeof(buff), "%d", val);

                break;  

            case 0x06: // 6 byte integer (48 bit)

                val2 = mbus_data_long_decode(record->data, 6);
        
                snprintf(buff, sizeof(buff), "%lu", val2);

                break;          

            case 0x07: // 8 byte integer (64 bit)

                val2 = mbus_data_long_decode(record->data, 8);
        
                snprintf(buff, sizeof(buff), "%lu", val2);

                break;          

            case 0x09: // 2 digit BCD (8 bit)
  
                val = mbus_data_bcd_decode(record->data, 1);  
                snprintf(buff, sizeof(buff), "%d", val);     
        
                break;
                
            case 0x0A: // 4 digit BCD (16 bit)
        
                val = mbus_data_bcd_decode(record->data, 2);  
                snprintf(buff, sizeof(buff), "%d", val);     

                break;

            case 0x0B: // 6 digit BCD (24 bit)

                val = mbus_data_bcd_decode(record->data, 3);  
                snprintf(buff, sizeof(buff), "%d", val);     

                break;
                
            case 0x0C: // 8 digit BCD (32 bit)

                val = mbus_data_bcd_decode(record->data, 4);  
                snprintf(buff, sizeof(buff), "%d", val);     

                break;
                                                                  
            case 0x0D: // variable length
                if(record->data_len <= 0xBF) {
                    mbus_data_str_decode(buff, record->data, record->data_len);
                    break;
                } /* FALLTHROUGH */

            default:
        
                snprintf(buff, sizeof(buff), "Unknown DIF (0x%.2x)", record->drh.dib.dif);
                break;
        }

        return buff;     
    }

    return NULL;
}
//------------------------------------------------------------------------------
// Return the unit description for a variable-length data record
//------------------------------------------------------------------------------
const char *
mbus_data_record_unit(mbus_data_record *record)
{
    static char buff[128];
    
    if (record)
    {
        snprintf(buff, sizeof(buff), "%s", mbus_vib_unit_lookup(&(record->drh.vib)));
                 
        return buff;    
    }
    
    return NULL;
}

//------------------------------------------------------------------------------
// Return the numerical value for a variable-length data record
//------------------------------------------------------------------------------
const char *
mbus_data_record_value(mbus_data_record *record)
{
    static char buff[128];
    
    if (record)
    {
        snprintf(buff, sizeof(buff), "%s", mbus_data_record_decode(record));
                 
        return buff;    
    }
    
    return NULL;
}

//------------------------------------------------------------------------------
// Return a string containing the function description
//------------------------------------------------------------------------------
const char *
mbus_data_record_function(mbus_data_record *record)
{
    static char buff[128];
    
    if (record)
    {
        switch (record->drh.dib.dif & 0x30)
        {
            case 0x00:
                snprintf(buff, sizeof(buff), "Instantaneous value");           
                break;

            case 0x10:
                snprintf(buff, sizeof(buff), "Maximum value");
                break;

            case 0x20:
                snprintf(buff, sizeof(buff), "Minimum value");
                break;

            case 0x30:
                snprintf(buff, sizeof(buff), "Value during error state");                       
                break;
        
            default:
                snprintf(buff, sizeof(buff), "unknown");                 
        }

        return buff;    
    }
    
    return NULL;
}

const char *
mbus_data_fixed_function(int status)
{
    static char buff[128];

    snprintf(buff, sizeof(buff), "%s",
            (status & MBUS_DATA_FIXED_STATUS_DATE_MASK) == MBUS_DATA_FIXED_STATUS_DATE_STORED ? 
            "Stored value" : "Actual value" );
    
    return buff;
}

//------------------------------------------------------------------------------
//
// PARSER FUNCTIONS
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PARSE M-BUS frame data structures from binary data.
//------------------------------------------------------------------------------
int
mbus_parse(mbus_frame *frame, u_char *data, size_t data_size)
{
    size_t i, len;

    if (frame && data && data_size > 0)
    {
	if (parse_debug)
	    printf("%s: Attempting to parse binary data [size = %d]\n", __PRETTY_FUNCTION__, data_size);

	if (parse_debug)
	    printf("%s: ", __PRETTY_FUNCTION__);

	for (i = 0; i < data_size && parse_debug; i++)
	{
	    printf("%.2X ", data[i] & 0xFF);
	}

	if (parse_debug)
	    printf("\n%s: done.\n", __PRETTY_FUNCTION__);

	switch (data[0])
	{
	    case MBUS_FRAME_ACK_START:

		// OK, got a valid ack frame, require no more data
		frame->start1   = data[0];
		frame->type = MBUS_FRAME_TYPE_ACK;
		return 0;
		//return MBUS_FRAME_BASE_SIZE_ACK - 1; // == 0

	    case MBUS_FRAME_SHORT_START:

		if (data_size < MBUS_FRAME_BASE_SIZE_SHORT)
		{
		    // OK, got a valid short packet start, but we need more data
		    return MBUS_FRAME_BASE_SIZE_SHORT - data_size;
		}

		if (data_size != MBUS_FRAME_BASE_SIZE_SHORT)
		{
		    // too much data... ?
		    return -2;
		}

		// init frame data structure
		frame->start1   = data[0];
		frame->control  = data[1];
		frame->address  = data[2];
		frame->checksum = data[3];
		frame->stop     = data[4];

		frame->type = MBUS_FRAME_TYPE_SHORT;

		// verify the frame
		if (mbus_frame_verify(frame) != 0)
		{
		    return -3;
		}

		// successfully parsed data
		return 0;

	    case MBUS_FRAME_LONG_START: // (also CONTROL)

		if (data_size < 3)
		{
		    // OK, got a valid long/control packet start, but we need
		    // more data to determine the length
		    return 3 - data_size;
		}

		// init frame data structure
		frame->start1   = data[0];
		frame->length1  = data[1];
		frame->length2  = data[2];

		if (frame->length1 != frame->length2)
		{
		    // not a valid M-bus frame
		    return -2;
		}

		// check length of packet:
		len = frame->length1;

		if (data_size < (size_t)(MBUS_FRAME_FIXED_SIZE_LONG + len))
		{
		    // OK, but we need more data
		    return MBUS_FRAME_FIXED_SIZE_LONG + len - data_size;
		}

		// we got the whole packet, continue parsing
		frame->start2   = data[3];
		frame->control  = data[4];
		frame->address  = data[5];
		frame->control_information = data[6];

		frame->data_size = len - 3;
		for (i = 0; i < frame->data_size; i++)
		{
		    frame->data[i] = data[7 + i];
		}

		frame->checksum = data[data_size-2]; // data[6 + frame->data_size + 1]
		frame->stop     = data[data_size-1]; // data[6 + frame->data_size + 2]

		if (frame->data_size == 0)
		{
		    frame->type = MBUS_FRAME_TYPE_CONTROL;
		}
		else
		{
		    frame->type = MBUS_FRAME_TYPE_LONG;
		}

		// verify the frame
		if (mbus_frame_verify(frame) != 0)
		{
		    return -3;
		}

		// successfully parsed data
		return 0;
	    default:
		// not a valid M-Bus frame header (start byte)
		return -4;
	}

    }

    return -1;
}


//------------------------------------------------------------------------------
// Parse the fixed-length data of a M-Bus frame
//------------------------------------------------------------------------------
int
mbus_data_fixed_parse(mbus_frame *frame, mbus_data_fixed *data)
{
    if (frame && data)
    {
	// copy the fixed-length data structure
	memcpy((void *)data, (void *)(frame->data), sizeof(mbus_data_fixed));

	return 0;
    }

    return -1;
}


//------------------------------------------------------------------------------
// Parse the variable-length data of a M-Bus frame
//------------------------------------------------------------------------------
int
mbus_data_variable_parse(mbus_frame *frame, mbus_data_variable *data)
{
    mbus_data_record *record;
    size_t i, j;

    if (frame && data)
    {
	// parse header
	data->nrecords = 0;
	i = sizeof(mbus_data_variable_header);
	if(frame->data_size < i)
	    return -1;

	// first copy the variable data fixed header
	memcpy((void *)&(data->header), (void *)(frame->data), i);

	data->record = NULL;

	while (i < frame->data_size)
	{
	    if ((record = mbus_data_record_new()) == NULL)
	    {
		// clean up...
		return (-2);
	    }

	    // read and parse DIB

	    // DIF
	    record->drh.dib.dif = frame->data[i];

	    if ((record->drh.dib.dif & 0xFF) == 0x0F || (record->drh.dib.dif & 0xFF) == 0x1F)
	    {
		i++;
		// just copy the remaining data as it is vendor specific
		record->data_len = frame->data_size - i;
		for (j = 0; j < record->data_len; j++)
		{
		    record->data[j] = frame->data[i++];
		}

		// append the record and move on to next one
		mbus_data_record_append(data, record);
		data->nrecords++;
		continue;
	    }

	    // calculate length of data record
	    record->data_len = record->drh.dib.dif & 0x07;

	    // read DIF extensions
	    record->drh.dib.ndife = 0;
	    while (frame->data[i] & MBUS_DIB_DIF_EXTENSION_BIT && record->drh.dib.ndife < NITEMS(record->drh.dib.dife))
	    {
		u_char dife = frame->data[i+1];
		record->drh.dib.dife[record->drh.dib.ndife] = dife;

		record->drh.dib.ndife++;
		i++;
	    }
	    i++;

	    // VIF
	    record->drh.vib.vif = frame->data[i];

	    // VIFE
	    record->drh.vib.nvife = 0;
	    while (frame->data[i] & MBUS_DIB_VIF_EXTENSION_BIT && record->drh.vib.nvife < NITEMS(record->drh.vib.vife))
	    {
		u_char vife = frame->data[i+1];
		record->drh.vib.vife[record->drh.vib.nvife] = vife;

		record->drh.vib.nvife++;
		i++;
	    }
	    i++;

	    // calculate data length
	    if((record->drh.dib.dif & 0x0D) == 0x0D)
	    {
		if(frame->data[i] <= 0xBF)
		    record->data_len = frame->data[i++];
		else if(frame->data[i] >= 0xC0 && frame->data[i] <= 0xCF)
		    record->data_len = (frame->data[i++] - 0xC0) * 2;
		else if(frame->data[i] >= 0xD0 && frame->data[i] <= 0xDF)
		    record->data_len = (frame->data[i++] - 0xD0) * 2;
		else if(frame->data[i] >= 0xE0 && frame->data[i] <= 0xEF)
		    record->data_len = frame->data[i++] - 0xE0;
		else if(frame->data[i] >= 0xF0 && frame->data[i] <= 0xFA)
		    record->data_len = frame->data[i++] - 0xF0;
	    }

	    // copy data
	    for (j = 0; j < record->data_len; j++)
	    {
		record->data[j] = frame->data[i++];
	    }

	    // append the record and move on to next one
	    mbus_data_record_append(data, record);
	    data->nrecords++;
	}

	return 0;
    }

    return -1;
}

//------------------------------------------------------------------------------
// Check the stype of the frame data (fixed or variable) and dispatch to the
// corresponding parser function.
//------------------------------------------------------------------------------
int
mbus_frame_data_parse(mbus_frame *frame, mbus_frame_data *data)
{
    if (frame && data && frame->data_size > 0)
    {
	if (frame->control_information == MBUS_CONTROL_INFO_RESP_FIXED)
	{
	    data->type = MBUS_DATA_TYPE_FIXED;
	    return mbus_data_fixed_parse(frame, &(data->data_fix));
	}

	if (frame->control_information == MBUS_CONTROL_INFO_RESP_VARIABLE)
	{
	    data->type = MBUS_DATA_TYPE_VARIABLE;
	    return mbus_data_variable_parse(frame, &(data->data_var));
	}
    }

    return -1;
}

//------------------------------------------------------------------------------
// Pack the M-bus frame into a binary string representation that can be sent 
// on the bus. The binary packet format is different for the different types
// of M-bus frames.
//------------------------------------------------------------------------------
int mbus_frame_pack(mbus_frame *frame, u_char *data, size_t data_size)
{
    size_t i, offset = 0;
    
    if (frame && data)
    {   
        if (mbus_frame_calc_length(frame) == -1)
        {
            return -2;
        }

        if (mbus_frame_calc_checksum(frame) == -1)
        {
            return -3;
        }
        
        switch (frame->type)
        {
            case MBUS_FRAME_TYPE_ACK:
            
                if (data_size < MBUS_FRAME_ACK_BASE_SIZE)
                {
                    return -4;
                }
        
                data[offset++] = frame->start1; 
        
                return offset;        

            case MBUS_FRAME_TYPE_SHORT:

                if (data_size < MBUS_FRAME_SHORT_BASE_SIZE)
                {
                    return -4;
                }
                
                data[offset++] = frame->start1;
                data[offset++] = frame->control;
                data[offset++] = frame->address;
                data[offset++] = frame->checksum;
                data[offset++] = frame->stop;
        
                return offset;        

            case MBUS_FRAME_TYPE_CONTROL:
        
                if (data_size < MBUS_FRAME_CONTROL_BASE_SIZE)
                {
                    return -4;
                }
                
                data[offset++] = frame->start1;
                data[offset++] = frame->length1;
                data[offset++] = frame->length2;
                data[offset++] = frame->start2;

                data[offset++] = frame->control;
                data[offset++] = frame->address;
                data[offset++] = frame->control_information;

                data[offset++] = frame->checksum;
                data[offset++] = frame->stop;    
        
                return offset;        

            case MBUS_FRAME_TYPE_LONG:

                if (data_size < frame->data_size + MBUS_FRAME_LONG_BASE_SIZE)
                {
                    return -4;
                }

                data[offset++] = frame->start1;
                data[offset++] = frame->length1;
                data[offset++] = frame->length2;
                data[offset++] = frame->start2;

                data[offset++] = frame->control;
                data[offset++] = frame->address;
                data[offset++] = frame->control_information;

                for (i = 0; i < frame->data_size; i++)
                {
                    data[offset++] = frame->data[i];
                }
                
                data[offset++] = frame->checksum;
                data[offset++] = frame->stop;    
        
                return offset;      
            
            default:
                return -5;
        }        
    }

    return -1;
}


//------------------------------------------------------------------------------
// pack the data stuctures into frame->data
//------------------------------------------------------------------------------
int
mbus_frame_internal_pack(mbus_frame *frame, mbus_frame_data *frame_data)
{
    mbus_data_record *record;
    int i, j;

    if (frame == NULL || frame_data == NULL)
        return -1;

    frame->data_size = 0;

    switch (frame_data->type)
    {
        case MBUS_DATA_TYPE_FIXED:

            //
            // pack fixed data structure
            //
            frame->data[frame->data_size++] = frame_data->data_fix.id_bcd[0];
            frame->data[frame->data_size++] = frame_data->data_fix.id_bcd[1];
            frame->data[frame->data_size++] = frame_data->data_fix.id_bcd[2];
            frame->data[frame->data_size++] = frame_data->data_fix.id_bcd[3];
            frame->data[frame->data_size++] = frame_data->data_fix.tx_cnt;
            frame->data[frame->data_size++] = frame_data->data_fix.status;
            frame->data[frame->data_size++] = frame_data->data_fix.cnt1_type;
            frame->data[frame->data_size++] = frame_data->data_fix.cnt2_type;
            frame->data[frame->data_size++] = frame_data->data_fix.cnt1_val[0];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt1_val[1];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt1_val[2];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt1_val[3];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt2_val[0];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt2_val[1];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt2_val[2];
            frame->data[frame->data_size++] = frame_data->data_fix.cnt2_val[3];

            break;

        case MBUS_DATA_TYPE_VARIABLE:

            //
            // first pack variable data structure header
            //
            frame->data[frame->data_size++] = frame_data->data_var.header.id_bcd[0];
            frame->data[frame->data_size++] = frame_data->data_var.header.id_bcd[1];
            frame->data[frame->data_size++] = frame_data->data_var.header.id_bcd[2];
            frame->data[frame->data_size++] = frame_data->data_var.header.id_bcd[3];
            frame->data[frame->data_size++] = frame_data->data_var.header.manufacturer[0];
            frame->data[frame->data_size++] = frame_data->data_var.header.manufacturer[1];
            frame->data[frame->data_size++] = frame_data->data_var.header.version;
            frame->data[frame->data_size++] = frame_data->data_var.header.medium;
            frame->data[frame->data_size++] = frame_data->data_var.header.access_no;
            frame->data[frame->data_size++] = frame_data->data_var.header.status;
            frame->data[frame->data_size++] = frame_data->data_var.header.signature[0];
            frame->data[frame->data_size++] = frame_data->data_var.header.signature[1];

            //
            // pack all data records
            //
            for (record = frame_data->data_var.record; record; record = record->next)
            {
                // pack DIF
                if (parse_debug)
                    printf("%s: packing DIF [%d]", __PRETTY_FUNCTION__, frame->data_size);
                frame->data[frame->data_size++] = record->drh.dib.dif;
                for (j = 0; j < record->drh.dib.ndife; j++)
                {
                    frame->data[frame->data_size++] = record->drh.dib.dife[j];
                }

                // pack VIF
                if (parse_debug)
                    printf("%s: packing VIF [%d]", __PRETTY_FUNCTION__, frame->data_size);
                frame->data[frame->data_size++] = record->drh.vib.vif;
                for (j = 0; j < record->drh.vib.nvife; j++)
                {
                    frame->data[frame->data_size++] = record->drh.vib.vife[j];
                }

                // pack data
                if (parse_debug) 
                    printf("%s: packing data [%d : %d]", __PRETTY_FUNCTION__, frame->data_size, record->data_len);
                for (j = 0; j < record->data_len; j++)
                {
                    frame->data[frame->data_size++] = record->data[j];
                }
            }         
            
            break;

        default:
            return -2;
    }

    return 0;
}

//------------------------------------------------------------------------------
//
// Print/Dump functions
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Dump frame in HEX on standard output
//------------------------------------------------------------------------------
int
mbus_frame_print(mbus_frame *frame)
{
    u_char data_buff[256];
    int len, i;
    
    if (frame)
    {
        if ((len = mbus_frame_pack(frame, data_buff, sizeof(data_buff))) == -1)
        {
            return -2;
        }
       
        printf("%s: Dumping M-Bus frame [type %d, %d bytes]: ", __PRETTY_FUNCTION__, frame->type, len);
        for (i = 0; i < len; i++)
        {
            printf("%.2X ", data_buff[i]);
        }
        printf("\n");
        
        return 0;
    }
    
    return -1;
}

int
mbus_frame_data_print(mbus_frame_data *data)
{
    if (data)
    {
        if (data->type == MBUS_DATA_TYPE_FIXED)
        {
            return mbus_data_fixed_print(&(data->data_fix));
        }
        
        if (data->type == MBUS_DATA_TYPE_VARIABLE)
        {
            return mbus_data_variable_print(&(data->data_var));
        }
    }
    
    return -1;
}

//------------------------------------------------------------------------------
// Print M-bus frame info to stdout
//------------------------------------------------------------------------------
int
mbus_data_variable_header_print(mbus_data_variable_header *header)
{
    if (header)
    {
        printf("%s: ID           = %d\n", __PRETTY_FUNCTION__, 
               mbus_data_bcd_decode(header->id_bcd, 4));

        printf("%s: Manufacturer = 0x%.2X%.2X\n", __PRETTY_FUNCTION__,
               header->manufacturer[1], header->manufacturer[0]);
               
        printf("%s: Manufacturer = %s\n", __PRETTY_FUNCTION__,
               mbus_decode_manufacturer(header->manufacturer[0], header->manufacturer[1]));               

        printf("%s: Version      = 0x%.2X\n", __PRETTY_FUNCTION__, header->version);
        printf("%s: Medium       = %s (0x%.2X)\n", __PRETTY_FUNCTION__, mbus_data_variable_medium_lookup(header->medium), header->medium);
        printf("%s: Access #     = 0x%.2X\n", __PRETTY_FUNCTION__, header->access_no);
        printf("%s: Status       = 0x%.2X\n", __PRETTY_FUNCTION__, header->status);
        printf("%s: Signature    = 0x%.2X%.2X\n", __PRETTY_FUNCTION__,
               header->signature[1], header->signature[0]);

    }
    
    return -1;
}

int
mbus_data_variable_print(mbus_data_variable *data)
{
    mbus_data_record *record;
    size_t i, j;
    
    if (data)
    {   
        mbus_data_variable_header_print(&(data->header));
    
        for (record = data->record; record; record = record->next)
        {
            // DIF
            printf("DIF           = %.2X\n", record->drh.dib.dif);
            printf("DIF.Extension = %s\n",  (record->drh.dib.dif & MBUS_DIB_DIF_EXTENSION_BIT) ? "Yes":"No");        
            printf("DIF.Function  = %s\n",  (record->drh.dib.dif & 0x30) ? "Minimum value" : "Instantaneous value" );
            printf("DIF.Data      = %.2X\n", record->drh.dib.dif & 0x0F);

            // VENDOR SPECIFIC
            if (record->drh.dib.dif == 0x0F || record->drh.dib.dif == 0x1F) //MBUS_DIB_DIF_VENDOR_SPECIFIC)
            {
                printf("%s: DEBUG: VENDOR DATA [size=%zd] = ", __PRETTY_FUNCTION__, record->data_len);
                for (j = 0; j < record->data_len; j++)
                {
                    printf("%.2X ", record->data[j]);        
                }
                printf("\n");
                continue;
            }
        
            // calculate length of data record
            printf("DATA LENGTH = %zd\n", record->data_len);

            // DIFE
            for (j = 0; j < record->drh.dib.ndife; j++)
            {
                u_char dife = record->drh.dib.dife[j];
                
                printf("DIFE[%zd]           = %.2X\n", j,  dife);
                printf("DIFE[%zd].Extension = %s\n",   j, (dife & MBUS_DIB_DIF_EXTENSION_BIT) ? "Yes" : "No");        
                printf("DIFE[%zd].Function  = %s\n",   j, (dife & 0x30) ? "Minimum value" : "Instantaneous value" );
                printf("DIFE[%zd].Data      = %.2X\n", j,  dife & 0x0F);            
            }
   
        }
    }
    
    return -1;
}

int
mbus_data_fixed_print(mbus_data_fixed *data)
{
    if (data)
    {    
        printf("%s: ID       = %d\n", __PRETTY_FUNCTION__, mbus_data_bcd_decode(data->id_bcd, 4));
        printf("%s: Access # = 0x%.2X\n", __PRETTY_FUNCTION__, data->tx_cnt);
        printf("%s: Status   = 0x%.2X\n", __PRETTY_FUNCTION__, data->status);
        printf("%s: Function = %s\n", __PRETTY_FUNCTION__, mbus_data_fixed_function(data->status));
        
        printf("%s: Medium1  = %s\n", __PRETTY_FUNCTION__, mbus_data_fixed_medium(data));
        printf("%s: Unit1    = %s\n", __PRETTY_FUNCTION__, mbus_data_fixed_unit(data->cnt1_type));
        if ((data->status & MBUS_DATA_FIXED_STATUS_FORMAT_MASK) == MBUS_DATA_FIXED_STATUS_FORMAT_BCD)
        {
            printf("%s: Counter1 = %d\n", __PRETTY_FUNCTION__, mbus_data_bcd_decode(data->cnt1_val, 4));
        }
        else
        {
            printf("%s: Counter1 = %d\n", __PRETTY_FUNCTION__, mbus_data_int_decode(data->cnt1_val, 4));
        }

        printf("%s: Medium2  = %s\n", __PRETTY_FUNCTION__, mbus_data_fixed_medium(data));
        printf("%s: Unit2    = %s\n", __PRETTY_FUNCTION__, mbus_data_fixed_unit(data->cnt2_type));
        if ((data->status & MBUS_DATA_FIXED_STATUS_FORMAT_MASK) == MBUS_DATA_FIXED_STATUS_FORMAT_BCD)
        {
            printf("%s: Counter2 = %d\n", __PRETTY_FUNCTION__, mbus_data_bcd_decode(data->cnt2_val, 4));
        }
        else
        {
            printf("%s: Counter2 = %d\n", __PRETTY_FUNCTION__, mbus_data_int_decode(data->cnt2_val, 4));        
        }          
    }
    
    return -1;
}

//------------------------------------------------------------------------------
//
// XML RELATED FUNCTIONS
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Generate XML for the variable-length data header
//------------------------------------------------------------------------------
char *
mbus_data_variable_header_xml(mbus_data_variable_header *header)
{
    static char buff[8192];
    size_t len = 0;
    int val;
    
    if (header)
    {
        len += snprintf(&buff[len], sizeof(buff) - len, "    <SlaveInformation>\n");

        val = mbus_data_bcd_decode(header->id_bcd, 4);
        
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Id>%d</Id>\n", val);
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Manufacturer>%s</Manufacturer>\n",
                mbus_decode_manufacturer(header->manufacturer[0], header->manufacturer[1]));               
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Version>%d</Version>\n", header->version);
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Medium>%s</Medium>\n",   mbus_data_variable_medium_lookup(header->medium));
        len += snprintf(&buff[len], sizeof(buff) - len, "        <AccessNumber>%d</AccessNumber>\n", header->access_no);
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Status>%.2X</Status>\n", header->status);
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Signature>%.2X%.2X</Signature>\n", header->signature[1], header->signature[0]);

        len += snprintf(&buff[len], sizeof(buff) - len, "    </SlaveInformation>\n\n");

        return buff;
    }
    
    return NULL;
}

//------------------------------------------------------------------------------
// Generate XML for variable-length data 
//------------------------------------------------------------------------------
char *
mbus_data_variable_xml(mbus_data_variable *data)
{
    mbus_data_record *record;
    static char buff[8192];
    size_t len = 0;
    size_t i;
    
    if (data)
    {
        len += snprintf(&buff[len], sizeof(buff) - len, "<MBusData>\n\n");
        
        len += snprintf(&buff[len], sizeof(buff) - len, "%s", mbus_data_variable_header_xml(&(data->header)));
    
        for (record = data->record, i = 0; record; record = record->next, i++)
        {
            if (record->drh.dib.dif == 0x0F || record->drh.dib.dif == 0x1F) //MBUS_DIB_DIF_VENDOR_SPECIFIC)
            {
                len += snprintf(&buff[len], sizeof(buff) - len, "    <DataRecord id=\"%zd\">\n", i);
                len += snprintf(&buff[len], sizeof(buff) - len, "        <Function>Manufacturer specific</Function>\n");
                len += snprintf(&buff[len], sizeof(buff) - len, "    </DataRecord>\n\n");
            }
            else
            {
                len += snprintf(&buff[len], sizeof(buff) - len, "    <DataRecord id=\"%zd\">\n", i);
                len += snprintf(&buff[len], sizeof(buff) - len, "        <Function>%s</Function>\n",
                                mbus_data_record_function(record));
                len += snprintf(&buff[len], sizeof(buff) - len, "        <Unit>%s</Unit>\n",
                                mbus_data_record_unit(record));
                len += snprintf(&buff[len], sizeof(buff) - len, "        <Value>%s</Value>\n",
                                mbus_data_record_value(record));
                len += snprintf(&buff[len], sizeof(buff) - len, "    </DataRecord>\n\n");
            }
        }
       
        len += snprintf(&buff[len], sizeof(buff) - len, "</MBusData>\n");

        return buff;
    }
    
    return NULL;
}

char *
mbus_data_fixed_xml(mbus_data_fixed *data)
{
    static char buff[8192];
    size_t len = 0;

    if (data)
    {
        len += snprintf(&buff[len], sizeof(buff) - len, "<MBusData>\n\n");
    
        len += snprintf(&buff[len], sizeof(buff) - len, "    <SlaveInformation>\n");
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Id>%d</Id>\n", mbus_data_bcd_decode(data->id_bcd, 4));
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Medium>%s</Medium>\n", mbus_data_fixed_medium(data));
        len += snprintf(&buff[len], sizeof(buff) - len, "        <AccessNumber>%d</AccessNumber>\n", data->tx_cnt);
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Status>%.2X</Status>\n", data->status);
        len += snprintf(&buff[len], sizeof(buff) - len, "    </SlaveInformation>\n\n");
             
        len += snprintf(&buff[len], sizeof(buff) - len, "    <DataRecord id=\"0\">\n");
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Function>%s</Function>\n", mbus_data_fixed_function(data->status));
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Unit>%s</Unit>\n",         mbus_data_fixed_unit(data->cnt1_type));
        if ((data->status & MBUS_DATA_FIXED_STATUS_FORMAT_MASK) == MBUS_DATA_FIXED_STATUS_FORMAT_BCD)
        {
            len += snprintf(&buff[len], sizeof(buff) - len, "        <Value>%d</Value>\n", mbus_data_bcd_decode(data->cnt1_val, 4));
        }
        else
        {
            len += snprintf(&buff[len], sizeof(buff) - len, "        <Value>%d</Value>\n", mbus_data_int_decode(data->cnt1_val, 4));
        }
        len += snprintf(&buff[len], sizeof(buff) - len, "    </DataRecord>\n\n");      

        len += snprintf(&buff[len], sizeof(buff) - len, "    <DataRecord id=\"1\">\n");
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Function>%s</Function>\n", mbus_data_fixed_function(data->status));
        len += snprintf(&buff[len], sizeof(buff) - len, "        <Unit>%s</Unit>\n",         mbus_data_fixed_unit(data->cnt2_type));
        if ((data->status & MBUS_DATA_FIXED_STATUS_FORMAT_MASK) == MBUS_DATA_FIXED_STATUS_FORMAT_BCD)
        {
            len += snprintf(&buff[len], sizeof(buff) - len, "        <Value>%d</Value>\n", mbus_data_bcd_decode(data->cnt2_val, 4));
        }
        else
        {
            len += snprintf(&buff[len], sizeof(buff) - len, "        <Value>%d</Value>\n", mbus_data_int_decode(data->cnt2_val, 4));
        }
        len += snprintf(&buff[len], sizeof(buff) - len, "    </DataRecord>\n\n");      

        len += snprintf(&buff[len], sizeof(buff) - len, "</MBusData>\n");

        return buff;
    }
    
    return NULL;
}

//* ----------------------------------------------------------------------------
// Return a string containing an XML representation of the M-BUS frame.
//------------------------------------------------------------------------------
char *
mbus_frame_data_xml(mbus_frame_data *data)
{
    if (data)
    {
        if (data->type == MBUS_DATA_TYPE_FIXED)
        {
            return mbus_data_fixed_xml(&(data->data_fix));
        }
        
        if (data->type == MBUS_DATA_TYPE_VARIABLE)
        {
            return mbus_data_variable_xml(&(data->data_var));
        }
    }
    
    return NULL;
}



//* ----------------------------------------------------------------------------
// Allocate and initialize a new frame data structure
//------------------------------------------------------------------------------
mbus_frame_data *
mbus_frame_data_new()
{
    mbus_frame_data *data;

    if ((data = (mbus_frame_data *)malloc(sizeof(mbus_frame_data))) == NULL)
    {
        return NULL;
    }
    data->data_var.record = NULL;

    return data;
}


//* ----------------------------------------------------------------------------
// Free up data associated with a frame data structure
//------------------------------------------------------------------------------
void
mbus_frame_data_free(mbus_frame_data *data)
{
    if (data)
    {
        if (data->data_var.record)
        {
            mbus_data_record_free(data->data_var.record); // free's up the whole list
        }

        free(data);
    }
}



//* ----------------------------------------------------------------------------
// Allocate and initialize a new variable data record
//------------------------------------------------------------------------------
mbus_data_record *
mbus_data_record_new()
{
    mbus_data_record *record;

    if ((record = (mbus_data_record *)malloc(sizeof(mbus_data_record))) == NULL)
    {
        return NULL;
    }

    record->next = NULL;
    return record;    
}

//* ----------------------------------------------------------------------------
// free up memory associated with a data record and all the subsequent records
// in its list (apply recursively)
//------------------------------------------------------------------------------
void
mbus_data_record_free(mbus_data_record *record)
{
    if (record)
    {
        mbus_data_record *next = record->next;

        free(record);

        if (next)
            mbus_data_record_free(next);
    }
}

//* ----------------------------------------------------------------------------
// Return a string containing an XML representation of the M-BUS frame.
//------------------------------------------------------------------------------
void
mbus_data_record_append(mbus_data_variable *data, mbus_data_record *record)
{
    mbus_data_record *iter;

    if (data && record)
    {
        if (data->record == NULL)
        {
            data->record = record;
        }
        else
        {
            // find the end of the list
            for (iter = data->record; iter->next; iter = iter->next);

            iter->next = record;
        }
    }
}

