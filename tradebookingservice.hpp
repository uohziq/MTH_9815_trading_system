/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
    Trade() = default;
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};

template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
  tradeId = _tradeId;
  price = _price;
  book = _book;
  quantity = _quantity;
  side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
  return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
  return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
  return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
  return side;
}


// will define later
template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingToExecutionListener;

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
    map<string, Trade<T> > trades;
    vector<ServiceListener<Trade<T> >*> listeners;
    TradeBookingConnector<T>* connector;
    TradeBookingToExecutionListener<T>* listener;
public:
    TradeBookingService();
    ~TradeBookingService() {} // set empty
    Trade<T>& GetData(string _key) { return trades[_key]; }
    void OnMessage(Trade<T>& _data);
    void AddListener(ServiceListener<Trade<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<Trade<T> >*>& GetListeners() const { return listeners; }
    TradeBookingConnector<T>* GetConnector() { return connector; }
    TradeBookingToExecutionListener<T>* GetListener() { return listener; }
    void BookTrade(Trade<T>& _trade);
};

template<typename T>
TradeBookingService<T>::TradeBookingService()
{
    trades = map<string, Trade<T> >();
    listeners = vector<ServiceListener<Trade<T> >*>();
    connector = new TradeBookingConnector<T>(this);
    listener = new TradeBookingToExecutionListener<T>(this);
}

template<typename T>
void TradeBookingService<T>::OnMessage(Trade<T>& _data)
{
    trades[_data.GetTradeId()] = _data;
    
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_data);
}

template<typename T>
void TradeBookingService<T>::BookTrade(Trade<T>& _trade)
{
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_trade);
}


template<typename T>
class TradeBookingConnector : public Connector<Trade<T> >
{
private:
    TradeBookingService<T>* service;
public:
    TradeBookingConnector(TradeBookingService<T>* _service) { service = _service; }
    ~TradeBookingConnector() {} // set empty
    void Publish(Trade<T>& _data) {} // set empty
    void Subscribe(ifstream& _data);
};

template<typename T>
void TradeBookingConnector<T>::Subscribe(ifstream& _data_in)
{
    string _line;
    while (getline(_data_in, _line))
    {
        stringstream _lineStream(_line);
        string _cell;
        vector<string> _cells;
        while (getline(_lineStream, _cell, ','))
            _cells.push_back(_cell);
        
        string _productId = _cells[0];
        string _tradeId = _cells[1];
        double _price = ConvertPrice(_cells[2]);
        string _book = _cells[3];
        long _quantity = stol(_cells[4]);
        Side _side;
        if (_cells[5] == "BUY") _side = BUY;
        else if (_cells[5] == "SELL") _side = SELL;
        T _product = GetBond(_productId);
        Trade<T> _trade(_product, _tradeId, _price, _book, _quantity, _side);
        service->OnMessage(_trade);
    }
}

/**
 * Trade Booking Service Listener subscribing data from Execution Service to Trading Booking Service.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingToExecutionListener : public ServiceListener<ExecutionOrder<T> >
{
private:
    TradeBookingService<T>* service;
    long count;
public:
    TradeBookingToExecutionListener(TradeBookingService<T>* _service);
    ~TradeBookingToExecutionListener() {} // set empty
    void ProcessAdd(ExecutionOrder<T>& _data);
    void ProcessRemove(ExecutionOrder<T>& _data) {} // set empty
    void ProcessUpdate(ExecutionOrder<T>& _data) {} // set empty
};

template<typename T>
TradeBookingToExecutionListener<T>::TradeBookingToExecutionListener(TradeBookingService<T>* _service)
{
    service = _service;
    count = 0;
}

template<typename T>
void TradeBookingToExecutionListener<T>::ProcessAdd(ExecutionOrder<T>& _data)
{
    count++;
    T _product = _data.GetProduct();
    PricingSide _pricingSide = _data.GetPricingSide();
    string _orderId = _data.GetOrderId();
    double _price = _data.GetPrice();
    long _visibleQuantity = _data.GetVisibleQuantity();
    long _hiddenQuantity = _data.GetHiddenQuantity();
    
    Side _side;
    switch (_pricingSide)
    {
        case BID:
            _side = SELL;
            break;
        case OFFER:
            _side = BUY;
            break;
    }
    string _book;
    switch (count % 3)
    {
        case 0:
            _book = "TRSY1";
            break;
        case 1:
            _book = "TRSY2";
            break;
        case 2:
            _book = "TRSY3";
            break;
    }
    long _quantity = _visibleQuantity + _hiddenQuantity;
    
    Trade<T> _trade(_product, _orderId, _price, _book, _quantity, _side);
    service->OnMessage(_trade);
    service->BookTrade(_trade);
}

#endif
