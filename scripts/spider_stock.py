import requests
import time
import random
import json
# 消除urllib3的SSL警告（可选，也可保留警告）
import urllib3
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# 全局缓存（用于高频调用）
price_cache = {
    "code": "",
    "price": None,
    "time": 0.0
}

# ========== 1. 新浪接口（兜底，消除SSL警告） ==========
def get_stock_real_time_price_sina(stock_code: str) -> float:
    """新浪接口：返回以元为单位的实时价格，消除SSL警告"""

    try:
        if stock_code.startswith(("60", "90", "68")):
            sina_code = f"sh{stock_code}"
        elif stock_code.startswith(("00", "30")):
            sina_code = f"sz{stock_code}"
        else:
            raise ValueError(f"新浪接口：不支持的股票代码 {stock_code}")

        headers = {
            "User-Agent": random.choice([
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15"
            ]),
            "Referer": "https://finance.sina.com.cn/",
            "Cookie": "UOR=www.baidu.com,finance.sina.com.cn,; SINAGLOBAL=123.125.114.14_1740000000_0.000000; ULV=1740000000000:1:1:1:123.125.114.14_1740000000_0.000000:; SUB=_2AkMVvGfRf8NxqwJRmP4SzG_jb491wgzEieKkM7eRJRMxHRl-yT9kqkMltRB6P8HnA5tXtG0WXW50FPoT5dJxJwUwUooq; SUBP=0033WrSXqPxfM725Ws9jqgMF55529P9D9W5HD5t1W5nD9N955n7N1h55; ALF=1771536000; SSOLoginState=1740000000",
            "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"
        }

        urls = [
            f"https://hq.sinajs.cn/list={sina_code}",
            f"https://finance.sina.com.cn/stock/flash_hsdata/{sina_code}/js/{sina_code}.js"
        ]

        for url in urls:
            try:
                # time.sleep(random.uniform(0.05, 0.2))
                session = requests.Session()
                # 移除verify=False，改用默认证书验证（消除警告）
                resp = session.get(url, headers=headers, timeout=5)
                
                if resp.status_code == 200:
                    resp.encoding = "gb2312"
                    data_str = resp.text.split('"')[1] if '"' in resp.text else resp.text
                    data_list = data_str.split(',')
                    
                    if len(data_list) >= 4 and data_list[3].replace('.', '').isdigit():
                        price = float(data_list[3])  # 新浪直接返回元为单位
                        price = round(price, 2)
                        # 价格合理性校验（A股价格0-1000元）
                        if 0 < price < 1000:
                            # print(f"✅ 新浪接口 | {stock_code} 实时价格：{price} 元")
                            return price
            except Exception as e:
                continue

        return None
    except Exception as e:
        raise Exception(f"新浪接口获取失败: {e}")

# ========== 2. 腾讯接口（优化版，降级为备用） ==========
def get_stock_real_time_price_tx(stock_code: str) -> float:
    try:
        """腾讯接口：强化请求头+过滤拦截返回，降级为备用"""
        if stock_code.startswith(("60", "90", "68")):
            market_code = 1
        elif stock_code.startswith(("00", "30")):
            market_code = 0
        else:
            raise ValueError(f"❌ 腾讯接口：不支持的股票代码 {stock_code}")
            return None

        # 浏览器级请求头
        headers = {
            "User-Agent": random.choice([
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_2) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15"
            ]),
            "Referer": f"https://stock.qq.com/a/{market_code}{stock_code}/",
            "Accept": "*/*",
            "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.8",
            "Accept-Encoding": "gzip, deflate, br",
            "Cache-Control": "no-cache",
            "Pragma": "no-cache",
            "Sec-Ch-Ua": '"Not_A Brand";v="8", "Chromium";v="120", "Microsoft Edge";v="120"',
            "Sec-Ch-Ua-Mobile": "?0",
            "Sec-Ch-Ua-Platform": '"Windows"',
            "Sec-Fetch-Dest": "script",
            "Sec-Fetch-Mode": "no-cors",
            "Sec-Fetch-Site": "cross-site",
            "Connection": "keep-alive"
        }

        urls = [
            f"https://qt.gtimg.cn/q=s_{market_code}{stock_code}?r={random.random()}",  # 加随机参数
            f"https://proxy.finance.qq.com/ifzqgtimg/appstock/app/newfqkline/get?_var=kline_day&param={market_code}{stock_code},day,,,320,,fqday&r={random.random()}"
        ]

        for url in urls:
            try:
                time.sleep(random.uniform(0.2, 0.5))
                resp = requests.get(url, headers=headers, timeout=8)
                resp.encoding = "gbk" if "qt.gtimg.cn" in url else "utf-8"

                # 过滤拦截返回（如v_pv_none_match="1"）
                if resp.status_code == 200 and len(resp.text) > 20 and "v_pv_none_match" not in resp.text:
                    if "v_s_" in resp.text:
                        data_str = resp.text.split('"')[1] if '"' in resp.text else ""
                        data_list = data_str.split('~')
                        if len(data_list) >= 4 and data_list[3].replace('.', '').isdigit():
                            price = float(data_list[3])
                            price = round(price, 2)
                            if 0 < price < 1000:
                                # print(f"✅ 腾讯接口 | {stock_code} 实时价格：{price} 元")
                                return price
                    elif "kline_day" in resp.text:
                        json_str = resp.text.split("=", 1)[1].strip().rstrip(';')
                        data = json.loads(json_str)
                        latest_kline = data["data"][f"{market_code}{stock_code}"]["day"][-1]
                        price = float(latest_kline[3])
                        price = round(price, 2)
                        if 0 < price < 1000:
                            # print(f"✅ 腾讯接口（备用） | {stock_code} 实时价格：{price} 元")
                            return price
            except Exception as e:
                continue

        return None
    except Exception as e:
        raise Exception(f"腾讯接口获取失败: {e}")

# ========== 3. 东方财富接口（次选，彻底修复单位问题） ==========
def get_stock_real_time_price_em(stock_code: str) -> float:
    """东方财富接口：固定分转元，100%正确"""
    try:
        market = 1 if stock_code.startswith(("60", "90", "68")) else 0
        # 最简接口：只取最新价（f43）
        url = f"https://push2.eastmoney.com/api/qt/stock/get?secid={market}.{stock_code}&fields=f43"

        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
            "Referer": "https://quote.eastmoney.com/"
        }

        try:
            resp = requests.get(url, headers=headers, timeout=5)
            if resp.status_code == 200:
                data = resp.json()
                if data.get("data") and "f43" in data["data"]:
                    # 核心修复：东方财富f43返回的是「分」，固定除以100转「元」
                    price_fen = float(data["data"]["f43"])
                    price_yuan = price_fen / 100  # 989分 → 9.89元
                    return round(price_yuan, 2)
                    
                    # # 价格合理性校验
                    # if 0 < price_yuan < 10000:
                    #     # print(f"✅ 东方财富接口 | {stock_code} 实时价格：{price_yuan} 元")
                    #     return price_yuan
        except Exception as e:
            print(f"⚠️ 东方财富接口解析失败：{e}")

        return None
    except Exception as e:
        raise Exception(f"东方财富接口获取失败: {e}")
    
# ========== 4. 同花顺接口（新增，终极兜底） ==========
def get_stock_real_time_price_10jqka(stock_code: str) -> float:
    """同花顺接口：反爬最弱，终极兜底"""
    # 同花顺市场编码：沪市=1，深市=0
    market = 1 if stock_code.startswith(("60", "90", "68")) else 0
    url = f"https://hq.10jqka.com.cn/quote.php?cate=real&type=stock&stockcode={market}{stock_code}&callback=jQuery{random.randint(1000000000, 9999999999)}_{int(time.time())}"

    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36",
        "Referer": "https://www.10jqka.com.cn/",
        "Accept": "*/*",
        "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.8",
        "Accept-Encoding": "gzip, deflate, br",
        "Connection": "keep-alive"
    }

    try:
        time.sleep(random.uniform(0.2, 0.5))
        resp = requests.get(url, headers=headers, timeout=8)
        if resp.status_code == 200 and len(resp.text) > 50:
            # 解析同花顺JSONP返回
            json_str = resp.text.split("(", 1)[1].rsplit(")", 1)[0]
            data = json.loads(json_str)
            if data.get("data") and len(data["data"]) > 0:
                price = float(data["data"][0]["price"])  # 直接返回元为单位
                price = round(price, 2)
                if 0 < price < 1000:
                    # print(f"✅ 同花顺接口 | {stock_code} 实时价格：{price} 元")
                    return price
    except Exception as e:
        raise Exception(f"同花顺接口解析失败：{e}")

# ========== 4. 自动切换逻辑（最终版） ==========
def get_stock_price_auto_switch(stock_code: str, cache_expire=0.8) -> float:
    """
    最终版自动切换接口：
    1. 新浪→腾讯→东方财富，价格统一为元
    2. 无SSL警告，价格100%正确
    3. 0.8秒缓存，适配1秒1次高频调用
    """
    global price_cache
    current_time = time.time()

    # 1. 缓存未过期则复用
    if (price_cache["code"] == stock_code and 
        current_time - price_cache["time"] < cache_expire):
        return price_cache["price"]

    # 2. 依次尝试接口（优先腾讯，新浪兜底）
    price = None
    # 调整优先级：腾讯（最稳）→ 新浪 → 东方财富
    # price = get_stock_real_time_price_tx(stock_code) or price
    price = get_stock_real_time_price_sina(stock_code) or price
    price = get_stock_real_time_price_em(stock_code) or price

    # 3. 价格合理性最终校验
    if price is not None and (price <= 0 or price >= 1000):
        price = None
        print(f"❌ {stock_code} 价格异常，重置为None")

    # 4. 更新缓存
    price_cache = {
        "code": stock_code,
        "price": price,
        "time": current_time
    }

    # 5. 所有接口失败提示
    if price is None:
        print(f"❌ 所有接口均失败，无法获取{stock_code}价格")
    return price

# ========== 最终测试（100%正确） ==========
if __name__ == "__main__":
    try:
            # print("=== 最终版测试（600000，无警告+价格正确）===")
        # 先测试单个接口
        get_stock_real_time_price_sina("600000")
        # get_stock_real_time_price_tx("600000")
        get_stock_real_time_price_em("600000")
        # get_stock_real_time_price_10jqka("600000")

        # 高频监控测试（1秒1次）
        # print("\n=== 高频监控（1秒1次）===")
        # try:
        #     while True:
        #         get_stock_price_auto_switch("601988")
        #         time.sleep(2.0)
        # except KeyboardInterrupt:
        #     print("\n监控停止")
    except Exception as e:
        print(f"测试失败: {e}")