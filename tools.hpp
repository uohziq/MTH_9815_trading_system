//
//  tools.hpp
//  tradingsystem
//
//  Created by Robert on 12/21/18.
//  Copyright © 2018 Zhou Robert Qi. All rights reserved.
//

#ifndef tools_hpp
#define tools_hpp

#include <iostream>
#include <string>
#include <chrono>

// #include "products.hpp"

using namespace std;
using namespace chrono;


// print the time stamp in specific format
string PrintTimeStamp()
{
    auto now = system_clock::now();
    auto _sec = chrono::time_point_cast<chrono::seconds>(now);
    auto _millisec = chrono::duration_cast<chrono::milliseconds>(now - _sec);
    
    auto _millisecCount = _millisec.count();
    string _milliString = to_string(_millisecCount);
    if (_millisecCount < 10) _milliString = "00" + _milliString;
    else if (_millisecCount < 100) _milliString = "0" + _milliString;
    
    time_t _timeT = system_clock::to_time_t(now);
    char _timeChar[24];
    strftime(_timeChar, 24, "%F %T", localtime(&_timeT));
    string _timeString = string(_timeChar) + "." + _milliString + " ";
    
    return _timeString;
}


// search online for these cusips
Bond GetBond(string _cusip)
{
    if (_cusip == "9128283H1")
    {
        Bond _bond("9128283H1", CUSIP, "US2Y", 0.01750, from_string("2019/11/30"));
        return _bond;
    }
    if (_cusip == "9128283L2")
    {
        Bond _bond("9128283L2", CUSIP, "US3Y", 0.01875, from_string("2020/12/15"));
        return _bond;
    }
    if (_cusip == "912828M80")
    {
        Bond _bond("912828M80", CUSIP, "US5Y", 0.02000, from_string("2022/11/30"));
        return _bond;
    }
    if (_cusip == "9128283J7")
    {
        Bond _bond("9128283J7", CUSIP, "US7Y", 0.02125, from_string("2024/11/30"));
        return _bond;
    }
    if (_cusip == "9128283F5")
    {
        Bond _bond("9128283F5", CUSIP, "US10Y", 0.02250, from_string("2027/12/15"));
        return _bond;
    }
    if (_cusip == "912810RZ3")
    {
        Bond _bond("912810RZ3", CUSIP, "US30Y", 0.02750, from_string("2047/12/15"));
        return _bond;
    }
    else // in case that there is no such cusip thus no return value
    {
        Bond _bond("999999999", CUSIP, "US99Y", 0.02750, from_string("2047/12/15"));
        return _bond;
    }
}


double ConvertPrice(string _price)
{
    int count = 0;
    string _price_int = "";
    string _price_32 = "";
    string _price_256 = "";
    for(int i = 0; i < _price.size(); ++i)
    {
        if (_price[i] == '-')
        {
            ++count;
            continue;
        }
        if(count == 0)
        {
            _price_int.push_back(_price[i]);
        }
        else if (count == 1 || count == 2)
        {
            _price_32.push_back(_price[i]);
            ++count;
        }
        else
        {
            if(_price[i] == '+')
            {
                _price_256.push_back('4');
            }
            else
            {
                _price_256.push_back(_price[i]);
            }
        }
    }
    
    double res = 0.;
    res += stod(_price_int);
    res += stod(_price_32) / 32.;
    res += stod(_price_256) / 256.;
    return res;
}

long GetMillisecond()
{
    auto _timePoint = system_clock::now();
    auto _sec = chrono::time_point_cast<chrono::seconds>(_timePoint);
    auto _millisec = chrono::duration_cast<chrono::milliseconds>(_timePoint - _sec);
    long _millisecCount = _millisec.count();
    return _millisecCount;
}

string ConvertPrice(double _doublePrice)
{
    int _doublePrice100 = floor(_doublePrice);
    int _doublePrice256 = floor((_doublePrice - _doublePrice100) * 256.0);
    int _doublePrice32 = floor(_doublePrice256 / 8.0);
    _doublePrice256 %= 8;
    
    string _stringPrice100 = to_string(_doublePrice100);
    string _stringPrice32 = to_string(_doublePrice32);
    string _stringPrice256 = to_string(_doublePrice256);
    
    if (_doublePrice32 < 10) _stringPrice32 = "0" + _stringPrice32;
    if (_doublePrice256 == 4) _stringPrice256 = "+";
    
    string res = _stringPrice100 + "-" + _stringPrice32 + _stringPrice256;
    return res;
}

vector<double> GenerateUniform(long N, long seed = 0)
{
    long m = 2147483647;
    long a = 39373;
    long q = m / a;
    long r = m % a;
    
    if (seed == 0) seed = time(0);
    seed = seed % m;
    vector<double> result;
    for (long i = 0; i < N; i++)
    {
        long k = seed / q;
        seed = a * (seed - k * q) - k * r;
        if (seed < 0) seed = seed + m;
        result.push_back(seed / (double)m);
    }
    return result;
}

string GenerateId()
{
    string _base = "1234567890QWERTYUIOPASDFGHJKLZXCVBNM";
    vector<double> _randoms = GenerateUniform(12, GetMillisecond());
    string _id = "";
    for (auto& r : _randoms)
    {
        int i = r * 36;
        _id.push_back(_base[i]);
    }
    return _id;
}

double GetPV01Value(string _cusip)
{
    double _pv01 = 0;
    if (_cusip == "9128283H1") _pv01 = 0.01948992;
    if (_cusip == "9128283L2") _pv01 = 0.02865304;
    if (_cusip == "912828M80") _pv01 = 0.04581119;
    if (_cusip == "9128283J7") _pv01 = 0.06127718;
    if (_cusip == "9128283F5") _pv01 = 0.08161449;
    if (_cusip == "912810RZ3") _pv01 = 0.15013155;
    return _pv01;
}

#endif /* tools_hpp */
