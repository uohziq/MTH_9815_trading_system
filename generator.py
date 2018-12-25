#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Dec 24 15:28:54 2018

@author: robert
"""


import os
import numpy as np
import pandas as pd


cusips = ['9128283H1', '9128283L2', '912828M80',
          '9128283J7', '9128283F5', '912810RZ3']

n = len(cusips)


def int_256(x):
    return (int(x * 256) / 256)


def transform(number):
    i_0 = int(number)
    number -= i_0
    number *= 32
    i_1 = int(number)
    number -= i_1
    i_2 = int(number * 8)

    if i_2 == 4:
        s = '00' + str(i_1) + '+'
    else:
        s = '00' + str(i_1) + str(i_2)
    s = s[-3:]

    s = str(i_0) + '-' + s
    return s


def gen_prices(path):
    res = pd.DataFrame()
    num = 1000  # should be 1000000  # even number
    ram = np.random.uniform(99, 101, num * n)
    ram = [int_256(x) for x in ram]

    tmp = []
    for i in cusips:
        tmp.extend([i] * num)
    res['cusip'] = tmp
    res['p1'] = ram

    tmp = [1/128, 1/64] * int(num * n / 2)
    res['p2'] = tmp
    res['p2'] += res['p1']

    res['p1'] = res['p1'].apply(transform)
    res['p2'] = res['p2'].apply(transform)

    filename = os.path.join(path, 'prices.txt')
    np.savetxt(filename, res.values, fmt='%s', delimiter=",")


def gen_trades(path):
    res = pd.DataFrame()
    num = 10  # even number

    tmp = []
    for i in cusips:
        tmp.extend([i] * num)
    res['cusip'] = tmp

    ram = np.random.uniform(0, 1E10, num * n)
    ram = [str(int(x)) for x in ram]
    res['traderID'] = ram

    ram = np.random.uniform(99, 101, num * n)
    ram = [int_256(x) for x in ram]
    res['price'] = ram
    res['price'] = res['price'].apply(transform)

    tmp = ['TRSY1', 'TRSY2', 'TRSY3'] * int(num * n / 3)
    res['books'] = tmp

    tmp = [1E7, 2E7, 3E7, 4E7, 5E7] * int(num * n / 5)
    res['quantity'] = tmp
    res['quantity'] = res['quantity'].apply(int)

    tmp = ['BUY', 'SELL'] * int(num * n / 2)
    res['side'] = tmp

    filename = os.path.join(path, 'trades.txt')
    np.savetxt(filename, res.values, fmt='%s', delimiter=",")


def gen_marketdata(path):
    res = pd.DataFrame()
    num = 1000  # should be 1000000  # even number
    mid = np.random.uniform(99, 101, int(num * n / 2))
    mid = [int_256(x) for x in mid]

    spread = [1, 2, 3, 4, 3, 2]
    spread = [x / 128 for x in spread]
    tmp = int(len(mid) / len(spread)) + 1
    spread = (spread * tmp)[:len(mid)]

    g = (lambda x: [x[0] - x[1] / 2, x[0] + x[1] / 2])
    prices = [g(x) for x in zip(mid, spread)]
    prices = [transform(i) for sub in prices for i in sub]

    tmp = []
    for i in cusips:
        tmp.extend([i] * num)
    res['cusip'] = tmp

    res['price'] = prices

    tmp = [1, 1, 2, 2, 3, 3, 4, 4, 5, 5]
    tmp *= int(len(res) / len(tmp) + 1)
    tmp = tmp[:len(res)]

    res['size'] = tmp
    res['size'] = res['size'].apply(lambda x: int(x * 1E7))

    tmp = ['BID', 'OFFER']
    tmp *= int(len(res) / len(tmp) + 1)
    tmp = tmp[:len(res)]

    res['side'] = tmp

    filename = os.path.join(path, 'marketdata.txt')
    np.savetxt(filename, res.values, fmt='%s', delimiter=",")


def gen_inquires(path):
    res = pd.DataFrame()
    num = 10  # even number

    ram = np.random.uniform(0, 1E10, num * n)
    ram = [str(int(x)) for x in ram]
    res['inquiryID'] = ram

    tmp = []
    for i in cusips:
        tmp.extend([i] * num)
    res['cusip'] = tmp

    tmp = ['BUY', 'SELL']
    tmp *= int(len(res) / len(tmp) + 1)
    tmp = tmp[:len(res)]
    res['side'] = tmp

    tmp = [1, 2, 3, 4, 5]
    tmp *= int(len(res) / len(tmp) + 1)
    tmp = tmp[:len(res)]

    res['size'] = tmp
    res['size'] = res['size'].apply(lambda x: int(x * 1E7))

    ram = np.random.uniform(99, 101, num * n)
    ram = [transform(int_256(x)) for x in ram]
    res['price'] = ram

    res['state'] = 'RECEIVED'

    filename = os.path.join(path, 'inquires.txt')
    np.savetxt(filename, res.values, fmt='%s', delimiter=",")


if __name__ == '__main__':
    path = os.path.abspath(__file__)
    path = os.path.abspath(os.path.join(path, '..'))
    gen_prices(path)
    gen_trades(path)
    gen_marketdata(path)
    gen_inquires(path)
