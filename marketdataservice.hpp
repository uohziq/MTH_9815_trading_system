/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"

using namespace std;

// have been defined somewhere else
// Side for market data
// enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // ctor for an order
    Order() = default; // Robert added, default version
  Order(double _price, long _quantity, PricingSide _side);
    ~Order() {} // set empty
  // Get the price on the order
  double GetPrice() const;

  // Get the quantity on the order
  long GetQuantity() const;

  // Get the side on the order
  PricingSide GetSide() const;

private:
  double price;
  long quantity;
  PricingSide side;

};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
    BidOffer() = default; // Robert added default constructor
  BidOffer(const Order &_bidOrder, const Order &_offerOrder);
    ~BidOffer() {} // set empty
  // Get the bid order
  const Order& GetBidOrder() const;

  // Get the offer order
  const Order& GetOfferOrder() const;

private:
  Order bidOrder;
  Order offerOrder;

};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // ctor for the order book
    OrderBook() = default; // Robert added default constructor
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);
    ~OrderBook() {} // set empty
  // Get the product
  const T& GetProduct() const;

  // Get the bid stack
  const vector<Order>& GetBidStack() const;

  // Get the offer stack
  const vector<Order>& GetOfferStack() const;

    // Robert added: Get the best bid/offer order
    const BidOffer& GetBidOffer() const;
private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};

template<typename T>
const BidOffer& OrderBook<T>::GetBidOffer() const
{
    double _bidPrice = INT_MIN;
    Order _bidOrder;
    for (auto b = bidStack.begin(); b != bidStack.end(); ++b)
    {
        double _price = b->GetPrice();
        if (_price > _bidPrice)
        {
            _bidPrice = _price;
            _bidOrder = *b;
        }
    }
    
    double _offerPrice = INT_MAX;
    Order _offerOrder;
    for (auto o = offerStack.begin(); o != offerStack.end(); ++o)
    {
        double _price = o->GetPrice();
        if (_price < _offerPrice)
        {
            _offerPrice = _price;
            _offerOrder = *o;
        }
    }
    
    auto res = BidOffer(_bidOrder, _offerOrder);
    return res;
}

Order::Order(double _price, long _quantity, PricingSide _side)
{
  price = _price;
  quantity = _quantity;
  side = _side;
}

double Order::GetPrice() const
{
  return price;
}
 
long Order::GetQuantity() const
{
  return quantity;
}
 
PricingSide Order::GetSide() const
{
  return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
  return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
  return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
  return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
  return offerStack;
}



// will define later
template<typename T>
class MarketDataConnector;

// Robert changed it from an abstruct class into a real one
/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
private:
    map<string, OrderBook<T> > orderBooks;
    vector<ServiceListener<OrderBook<T> >*> listeners;
    MarketDataConnector<T>* connector;
    int bookDepth;
public:
    MarketDataService();
    ~MarketDataService() {} // set empty
    OrderBook<T>& GetData(string _key) { return orderBooks[_key]; }
    void OnMessage(OrderBook<T>& _data);
    void AddListener(ServiceListener<OrderBook<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<OrderBook<T> >*>& GetListeners() const { return listeners; }
    MarketDataConnector<T>* GetConnector() { return connector; }
    int GetBookDepth() const { return bookDepth; }
    // Get the best bid/offer order
    const BidOffer& GetBestBidOffer(const string &productId) { return orderBooks[productId].GetBidOffer(); }
    // Aggregate the order book
    const OrderBook<T>& AggregateDepth(const string &productId) ;
};

template<typename T>
MarketDataService<T>::MarketDataService()
{
    orderBooks = map<string, OrderBook<T> >();
    listeners = vector<ServiceListener<OrderBook<T> >*>();
    connector = new MarketDataConnector<T>(this);
    bookDepth = 5;
}

template<typename T>
void MarketDataService<T>::OnMessage(OrderBook<T>& _data)
{
    orderBooks[_data.GetProduct().GetProductId()] = _data;
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_data);
}

template<typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string& productId)
{
    T& _product = orderBooks[productId].GetProduct();
    
    vector<Order>& _bidStackFrom = orderBooks[productId].GetBidStack();
    unordered_map<double, long> _bidHashTable;
    for (auto b = _bidStackFrom.begin(); b != _bidStackFrom.end(); ++b)
    {
        double _price = b->GetPrice();
        long _quantity = b->GetQuantity();
        _bidHashTable[_price] += _quantity;
    }
    vector<Order> _bidStackTo;
    for (auto p = _bidHashTable.begin(); p!= _bidHashTable.end(); ++p)
    {
        Order _bidOrder = Order(p->first, p->second, BID);
        _bidStackTo.push_back(_bidOrder);
    }
    
    vector<Order>& _offerStackFrom = orderBooks[productId].GetOfferStack();
    unordered_map<double, long> _offerHashTable;
    for (auto o = _offerStackFrom.begin(); o != _offerStackFrom.end(); ++o)
    {
        double _price = o->GetPrice();
        long _quantity = o->GetQuantity();
        _bidHashTable[_price] += _quantity;
    }
    vector<Order> _offerStackTo;
    for (auto p = _offerHashTable.begin(); p != _offerHashTable.end(); ++p)
    {
        Order _offerOrder = Order(p->first, p->second, OFFER);
        _offerStackTo.push_back(_offerOrder);
    }
    
    return OrderBook<T>(_product, _bidStackTo, _offerStackTo);
}


template<typename T>
class MarketDataConnector : public Connector<OrderBook<T> >
{
private:
    MarketDataService<T>* service;
public:
    // Connector and Destructor
    MarketDataConnector(MarketDataService<T>* _service) { service = _service; }
    ~MarketDataConnector() {} // set empty
    void Publish(OrderBook<T>& _data) {} // set empty
    void Subscribe(ifstream& _data);
};

template<typename T>
void MarketDataConnector<T>::Subscribe(ifstream& _data_in)
{
    int _bookDepth = service->GetBookDepth();
    int _thread = _bookDepth * 2;
    long _count = 0;
    vector<Order> _bidStack;
    vector<Order> _offerStack;
    string _line;
    while (getline(_data_in, _line))
    {
        string _productId;
        
        stringstream _lineStream(_line);
        string _cell;
        vector<string> _cells;
        while (getline(_lineStream, _cell, ','))
            _cells.push_back(_cell);
        
        _productId = _cells[0];
        double _price = ConvertPrice(_cells[1]);
        long _quantity = stol(_cells[2]);
        PricingSide _side;
        if (_cells[3] == "BID") _side = BID;
        else if (_cells[3] == "OFFER") _side = OFFER;
        Order _order(_price, _quantity, _side);
        switch (_side)
        {
            case BID:
                _bidStack.push_back(_order);
                break;
            case OFFER:
                _offerStack.push_back(_order);
                break;
        }
        
        _count++;
        if (_count % _thread == 0)
        {
            T _product = GetBond(_productId);
            OrderBook<T> _orderBook(_product, _bidStack, _offerStack);
            service->OnMessage(_orderBook);
            
            _bidStack = vector<Order>();
            _offerStack = vector<Order>();
        }
    }
}


#endif
