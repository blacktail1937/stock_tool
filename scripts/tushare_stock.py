import tushare as ts
import datetime
import pandas as pd

# 1. 初始化Tushare Pro（替换为你的token，免费注册即可获取）
ts.set_token("ebe61d00af20e4e3900b2da2b283d1677d894db3a5196f0d39c0b9c8")
pro = ts.pro_api()

def get_stock_non_adj_price(stock_code: str, trade_date: str = None):
    """
    获取单只股票的非复权日线价格（120免费积分权限可用）
    :param stock_code: 股票代码，如"600000.SH"（必须带.SH/.SZ后缀）
    :param trade_date: 日期，格式YYYYMMDD，默认取最新交易日
    :return: 非复权价格（收盘价），失败返回None
    """

    try:
        if stock_code.startswith(("60", "90", "68")):
            stock_code = f"{stock_code}.SH"
        elif stock_code.startswith(("00", "30")):
            stock_code = f"{stock_code}.SZ"

        df = pro.daily(ts_code=stock_code, start_date='20260213', end_date='20260214')

        # # daily接口：非复权日线，120积分唯一可用的股票行情接口
        # df = pro.daily(
        #     ts_code=stock_code,
        #     start_date=trade_date,
        #     end_date=trade_date
        # )
        # print(df)
        if df.empty:
            raise ValueError(f"{trade_date} 非交易日，或股票代码错误")
        
        # 提取核心价格字段（非复权）
        non_adj_close = df.iloc[0]["close"]  # 非复权收盘价
        non_adj_open = df.iloc[0]["open"]    # 非复权开盘价
        non_adj_high = df.iloc[0]["high"]    # 非复权最高价
        non_adj_low = df.iloc[0]["low"]      # 非复权最低价
        
        # 打印结果
        print(f"股票：{stock_code} | 日期：{trade_date}")
        print(f"非复权开盘价：{non_adj_open} 元")
        print(f"非复权最高价：{non_adj_high} 元")
        print(f"非复权最低价：{non_adj_low} 元")
        print(f"非复权收盘价：{non_adj_close} 元")
        
        return non_adj_close  # 返回收盘价作为核心价格
    
    except Exception as e:
        # 120积分常见报错：权限不足/频次超限/日期错误
        raise Exception(f"获取失败：{e}")

# 2. 调用示例：获取浦发银行最新非复权价格
if __name__ == "__main__":
    # 注意：股票代码必须带.SH（沪市）/.SZ（深市）
    latest_price = get_stock_non_adj_price("000776.SZ")
    if latest_price:
        print(f"\n最终结果：浦发银行最新非复权收盘价 = {latest_price} 元")

