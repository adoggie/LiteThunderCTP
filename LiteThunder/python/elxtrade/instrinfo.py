# -*- coding: utf-8 -*-


_timerule = {
    'DAY':          {
        'auction_time':      [85900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000
        },
        'time_mapping':      {
            85900:  90000,
            101500: 101459,
            113000: 112959,
            150000: 145959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000)
        ]
    },

    'SHFE':         {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000,
            (205845, 205915): 205900,
            (205945, 205959): 210000,
            (10001, 10015):   10000
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            10000:  5959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 235959), (0, 10000)
        ]
    },

    'SHFE2':        {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000,
            (205845, 205915): 205900,
            (205945, 205959): 210000,
            (230001, 230015): 230000
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            230000: 225959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 230000)
        ]
    },

    'SHFE3':        {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000,
            (205845, 205915): 205900,
            (205945, 205959): 210000,
            (23001, 23015):   23000
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            23000:  22959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 235959), (0, 23000)
        ]
    },

    'DCE_20190401': {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000,
            (205845, 205915): 205900,
            (205945, 205959): 210000,
            (230001, 230015): 230000
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            230000: 225959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 230000)
        ]
    },
    'DCE':          {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000,
            (205845, 205915): 205900,
            (205945, 205959): 210000,
            (233001, 233015): 233000
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            233000: 232959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 233000)
        ]
    },

    'DCEold':       {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85915):   85900,
            (85945, 85959):   90000,
            (101501, 101515): 101500,
            (102945, 102959): 103000,
            (113001, 113015): 113000,
            (132945, 132959): 133000,
            (150001, 150015): 150000,
            (205845, 205915): 205900,
            (205945, 205959): 210000,
            (23001, 23015):   23000
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            23000:  22959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 235959), (0, 23000)
        ]
    },

    'CZCE':         {
        'auction_time':      [85900, 205900],
        'tick_time_mapping': {
            (85845, 85859):   85900, (85901, 85915): 85900,
            (85945, 85959):   90000,
            (101501, 101730): 101500,
            (102900, 102959): 103000,
            (113001, 113200): 113000,
            (132900, 132959): 133000,
            (150001, 150200): 150000,
        },
        'time_mapping':      {
            85900:  90000,
            205900: 210000,
            101500: 101459,
            113000: 112959,
            150000: 145959,
            233000: 232959
        },
        'trading_period':    [
            (90000, 101500), (103000, 113000), (133000, 150000),
            (210000, 233000)
        ]
    },

    'CFFEX':        {
        'auction_time':      [91400],
        'tick_time_mapping': {

        },
        'time_mapping':      {
            91400:  91500,
            113000: 112959,
            151500: 151459
        },
        'trading_period':    [
            (91500, 113000), (130000, 151500)
        ]
    },

    'CFFEX2020':    {
        'auction_time':      [92900],
        'tick_time_mapping': {

        },
        'time_mapping':      {
            92900:  93000,
            113000: 112959,
            151500: 151459
        },
        'trading_period':    [
            (93000, 113000), (130000, 151500)
        ]
    },

    'CFFEX2016':    {
        'auction_time':      [92900],
        'tick_time_mapping': {

        },
        'time_mapping':      {
            92900:  93000,
            113000: 112959,
            150000: 145959
        },
        'trading_period':    [
            (93000, 113000), (130000, 150000)
        ]
    }
}

INSTRINFO = {
    # CFFEX
    'IF': {
        'exchange':     'CFFEX',
        'multiplier':   300,
        'ticksize':     0.2,
        'get_timerule': lambda dn, tn: _timerule['CFFEX'] \
            if dn < 20160000 else _timerule['CFFEX2016']
    },
    'IC': {
        'exchange':     'CFFEX',
        'multiplier':   200,
        'ticksize':     0.2,
        'get_timerule': lambda dn, tn: _timerule['CFFEX'] \
            if dn < 20160000 else _timerule['CFFEX2016']
    },
    'IH': {
        'exchange':     'CFFEX',
        'multiplier':   300,
        'ticksize':     0.2,
        'get_timerule': lambda dn, tn: _timerule['CFFEX'] \
            if dn < 20160000 else _timerule['CFFEX2016']
    },
    'T':  {
        'exchange':     'CFFEX',
        'multipler':    10000,
        'ticksize':     0.005,
        'get_timerule': lambda dn, tn: _timerule['CFFEX'] \
            if dn < 20200720 else _timerule['CFFEX2020']
    },
    'TF': {
        'exchange':     'CFFEX',
        'multipler':    10000,
        'ticksize':     0.005,
        'get_timerule': lambda dn, tn: _timerule['CFFEX'] \
            if dn < 20200720 else _timerule['CFFEX2020']
    },
    # DCE

    'j':  {
        'exchange':     'DCE',
        'multiplier':   100,
        'ticksize':     0.5,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'jm': {
        'exchange':     'DCE',
        'multiplier':   60,
        'ticksize':     0.5,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'l':  {
        'exchange':     'DCE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['DAY'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'pp': {
        'exchange':     'DCE',
        'multiplier':   5,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DAY'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'a':  {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'b':  {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'eg': {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DAY'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'c':  {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DAY'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'v':  {
        'exchange':     'DCE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['DAY'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },

    'y':  {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     2,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'p':  {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     2,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'm':  {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'i':  {
        'exchange':     'DCE',
        'multiplier':   100,
        'ticksize':     0.5,
        #        'get_timerule': lambda dn, tn: _timerule['DCEold'] \
        #            if dn < 20150508 or (dn == 20150508 and tn < 180000) \
        #            else _timerule['DCE']
        'get_timerule': lambda dn, tn: _timerule['DCE'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'jd': {
        'exchange':     'DCE',
        'multiplier':   5,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DAY']
    },
    'cs': {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DAY'] \
            if dn < 20190329 or (dn == 20190329 and tn < 180000) \
            else _timerule['DCE_20190401']
    },
    'rr': {
        'exchange':     'DCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DCE_20190401']
    },
    'eb': {
        'exchange':     'DCE',
        'multiplier':   5,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DCE_20190401']
    },

    'pg': {
        'exchange':     'DCE',
        'multiplier':   20,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['DCE_20190401']
    },

    'lh': {
        'exchange':     'DCE',
        'multiplier':   16,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['DCE_20190401']
    },

    # CZCE
    'CJ': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'FG': {
        'exchange':     'CZCE',
        'multiplier':   20,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'TA': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'CF': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'SR': {
        'exchange':     'CZCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'RM': {
        'exchange':     'CZCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'AP': {
        'exchange':     'CZCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'SM': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'SF': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'ZC': {
        'exchange':     'CZCE',
        'multiplier':   100,
        'ticksize':     0.2,
        'get_timerule': lambda dn, tn: _timerule['CZCE'],
        'former_name':  'TC'
    },
    'TC': {
        'multiplier': 200
    },
    'MA': {
        'exchange':     'CZCE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE'],
        'former_name':  'ME'
    },
    'ME': {
        'multiplier': 50
    },
    'OI': {
        'exchange':     'CZCE',
        'multiplier':   10,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['CZCE'],
        'former_name':  'RO'
    },
    'RO': {
        'multiplier': 5
    },
    'UR': {
        'exchange':     'CZCE',
        'multiplier':   20,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'CY': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'SA': {
        'exchange':     'CZCE',
        'multiplier':   20,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'PF': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },
    'PK': {
        'exchange':     'CZCE',
        'multiplier':   5,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['CZCE']
    },

    # SHFE
    'bu': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['SHFE'] \
            if dn < 20160503 or (dn == 20160503 and tn < 180000) \
            else _timerule['SHFE2']
    },
    'fu': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['SHFE'] \
            if dn < 20160503 or (dn == 20160503 and tn < 180000) \
            else _timerule['SHFE2']
    },

    'hc': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['SHFE'] \
            if dn < 20160503 or (dn == 20160503 and tn < 180000) \
            else _timerule['SHFE2']
    },
    'cu': {
        'exchange':     'SHFE',
        'multiplier':   5,
        'ticksize':     10,
        'get_timerule': lambda dn, tn: _timerule['SHFE'],
    },

    'sc': {
        'exchange':     'SHFE',
        'multiplier':   1000,
        'ticksize':     0.1,
        'get_timerule': lambda dn, tn: _timerule['SHFE3'],
    },
    'nr': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['SHFE2'],
    },
    'al': {
        'exchange':     'SHFE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['SHFE'],
    },
    'zn': {
        'exchange':     'SHFE',
        'multiplier':   5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['SHFE'],
    },
    'ni': {
        'exchange':     'SHFE',
        'multiplier':   1,
        'ticksize':     10,
        'get_timerule': lambda dn, tn: _timerule['SHFE'],
    },
    'rb': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['SHFE'] \
            if dn < 20160503 or (dn == 20160503 and tn < 180000) \
            else _timerule['SHFE2']
    },
    'ru': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['SHFE2'],
    },
    'sp': {
        'exchange':     'SHFE',
        'multipler':    10,
        'ticksize':     2,
        'get_timerule': lambda dn, tn: _timerule['SHFE2'],
    },

    'au': {
        'exchange':     'SHFE',
        'multiplier':   1000,
        'ticksize':     0.02,
        'get_timerule': lambda dn, tn: _timerule['SHFE3'],
    },
    'ag': {
        'exchange':     'SHFE',
        'multipler':    15,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['SHFE3'],
    },
    'pb': {
        'exchange':     'SHFE',
        'multipler':    5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['SHFE3'],
    },
    'sn': {
        'exchange':     'SHFE',
        'multipler':    1,
        'ticksize':     10,
        'get_timerule': lambda dn, tn: _timerule['SHFE3'],
    },
    'ss': {
        'exchange':     'SHFE',
        'multipler':    5,
        'ticksize':     5,
        'get_timerule': lambda dn, tn: _timerule['SHFE'],
    },
    'lu': {
        'exchange':     'SHFE',
        'multiplier':   10,
        'ticksize':     1,
        'get_timerule': lambda dn, tn: _timerule['SHFE2'],
    },

}
