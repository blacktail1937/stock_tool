import time

import akshare as ak
import pandas as pd

def get_mainboard_stock_list_akshare():
    """
    使用akshare获取A股所有股票，并根据代码规则筛选主板股票
    主板规则：
    - 上交所主板：代码以600、601、603开头，且长度为6位
    - 深交所主板：代码以000开头，且长度为6位
    """

    try:
        stock_list = ak.stock_info_a_code_name()
        mainboard_stocks = []
        for _, row in stock_list.iterrows():
            code = row['code']
            name = row['name']
            
            if code.startswith(('60')) and len(code) == 6:
                mainboard_stocks.append({'code': code, 'name': name,'market':'沪A','border':'主板'})
            elif code.startswith(('00')) and len(code) == 6:
                mainboard_stocks.append({'code': code, 'name': name,'market':'深A','border':'主板'})

        
        return mainboard_stocks
    except Exception as e:
        raise Exception(f"获取主板股票列表失败: {e}")

def get_stock_list_akshare():
    """
    使用akshare获取A股所有股票列表
    """
    try:
        stock_list = ak.stock_info_a_code_name()

        # print(f"接口总共获取到 {len(stock_list)} 只股票")

        stock_list2 = []
        for _, row in stock_list.iterrows():
            code = str(row['code'])
            name = str(row['name'])

            if code.startswith(('60')):
                stock_list2.append({'code': code, 'name': name,'market':'沪A','border':'主板'})
            elif code.startswith(('68')):
                stock_list2.append({'code': code, 'name': name,'market':'沪A','border':'科创板'})
            elif code.startswith(('00')):
                stock_list2.append({'code': code, 'name': name,'market':'深A','border':'主板'})
            elif code.startswith(('30')):
                stock_list2.append({'code': code, 'name': name,'market':'深A','border':'创业板'})
            elif code.startswith(('92')):
                stock_list2.append({'code': code, 'name': name,'market':'北交所','border':'无'})

        # print(f"总共获取到 {len(stock_list2)} 只股票")

        return stock_list2
    except Exception as e:
        raise Exception(f"获取股票列表失败: {e}")

def get_stock_price_xq(stock_code):
    """
    使用雪球接口获取单只股票实时行情
    :param stock_code: 股票代码，如 "600000"（不需要带市场前缀）
    """
    # print(f"【雪球接口】正在获取 {stock_code} 的实时行情...")

    try:
        # 拼接带市场前缀的代码（沪市SH，深市SZ）
        if stock_code.startswith('6'):
            symbol = f"SH{stock_code}"
        else:
            symbol = f"SZ{stock_code}"

        # 调用雪球个股实时行情接口
        df = ak.stock_individual_spot_xq(symbol=symbol)

        if df.empty:
            raise Exception("未获取到数据")

        return __dataFrame_to_dict(df)

    except Exception as e:
        raise Exception(f"获取失败: {e}")


def get_stock_price_em(stock_code):
    """
    使用东方财富接口获取单只股票实时行情
    :param stock_code: 股票代码，如 "600000"（不需要带市场前缀）
    """
    # print(f"【东方财富接口】正在获取 {stock_code} 的实时行情...")

    try:
        # 调用东方财富个股实时行情接口
        df = ak.stock_individual_info_em(symbol=stock_code)

        if df.empty:
            raise Exception("未获取到数据")


        return __dataFrame_to_dict(df)

    except Exception as e:
        raise Exception(f"获取失败: {e}")

def get_stock_prices_em(stock_codes):
    # 获取所有A股的实时行情数据
    all_stocks_spot_df = ak.stock_zh_a_spot_em()

    # 从全部数据中筛选出你需要的股票
    # 注意：DataFrame中的代码列名通常是 '代码'
    my_stocks_data = all_stocks_spot_df[all_stocks_spot_df['代码'].isin(stock_codes)]

    # 打印出来看看，包含最新价、涨跌幅等丰富信息
    return my_stocks_data.head()
    
def __dataFrame_to_dict(df:pd.DataFrame):
    """
    将DataFrame转换为字典dict[str,str]
    """
    try:
        result = {}

        for _,row in df.iterrows():            
            key=str(row['item'])
            value=str(row['value'])

            if pd.isna(value):
                result[key] = 'NaN'
            elif isinstance(value, (int, str,bool)):
                result[key] = str(value)
            elif isinstance(value, (float,pd.core.arrays.floating.Float64Dtype)):
                result[key] = repr(value)
            else:
                result[key] = str(value)

        return result
    
    except Exception as e:
        raise Exception(f"DataFrame转换为字典失败: {e}")

def add(a,b):
    return a+b

if __name__ == "__main__":
    # print(get_stock_price_xq("000776"))
    # 假设这是你持仓的股票代码列表
    my_stock_codes = ['000001', '600519', '300750']

    print(get_stock_prices_em(my_stock_codes))