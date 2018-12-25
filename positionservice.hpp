/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
    Position() = default;
  Position(const T &_product);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string &book);

  // Get the aggregate position
  long GetAggregatePosition();
    // Robert added
    map<string, long> GetPositions() { return positions; }
    vector<string> ToStrings() const;
    void AddPosition(string& _book, long _position) { positions[_book] += _position; }
private:
  T product;
  map<string,long> positions;

};

template<typename T>
Position<T>::Position(const T &_product) :
  product(_product)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
    long aggregatePosition = 0;
    for (auto& p : positions)
        aggregatePosition += p.second;
    return aggregatePosition;
}

template<typename T>
vector<string> Position<T>::ToStrings() const
{
    string _product = product.GetProductId();
    vector<string> _positions;
    for (auto& p : positions)
    {
        string _book = p.first;
        string _position = to_string(p.second);
        _positions.push_back(_book);
        _positions.push_back(_position);
    }
    
    vector<string> res;
    res.push_back(_product);
    res.insert(res.end(), _positions.begin(), _positions.end());
    return res;
}


// will define later
template<typename T>
class PositionToTradeBookingListener;

template<typename T>
class PositionService : public Service<string, Position<T> >
{
private:
    map<string, Position<T> > positions;
    vector<ServiceListener<Position<T> >*> listeners;
    PositionToTradeBookingListener<T>* listener;
public:
    PositionService();
    ~PositionService() {} // set empty
    Position<T>& GetData(string _key) { return positions[_key]; }
    void OnMessage(Position<T>& _data) { positions[_data.GetProduct().GetProductId()] = _data; }
    void AddListener(ServiceListener<Position<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<Position<T> >*>& GetListeners() const { return listeners; }
    PositionToTradeBookingListener<T>* GetListener() { return listener; }
    virtual void AddTrade(const Trade<T>& _trade);
};

template<typename T>
PositionService<T>::PositionService()
{
    positions = map<string, Position<T> >();
    listeners = vector<ServiceListener<Position<T> >*>();
    listener = new PositionToTradeBookingListener<T>(this);
}

template<typename T>
void PositionService<T>::AddTrade(const Trade<T>& _trade)
{
    T _product = _trade.GetProduct();
    string _productId = _product.GetProductId();
    double _price = _trade.GetPrice();
    string _book = _trade.GetBook();
    long _quantity = _trade.GetQuantity();
    Side _side = _trade.GetSide();
    Position<T> _positionTo(_product);
    switch (_side)
    {
        case BUY:
            _positionTo.AddPosition(_book, _quantity);
            break;
        case SELL:
            _positionTo.AddPosition(_book, -_quantity);
            break;
    }
    
    Position<T> _positionFrom = positions[_productId];
    map <string, long> _positionMap = _positionFrom.GetPositions();
    for (auto p = _positionMap.begin(); p != _positionMap.end(); ++p)
    {
        _book = p->first;
        _quantity = p->second;
        _positionTo.AddPosition(_book, _quantity);
    }
    positions[_productId] = _positionTo;
    
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_positionTo);
}


template<typename T>
class PositionToTradeBookingListener : public ServiceListener<Trade<T> >
{
private:
    PositionService<T>* service;
public:
    PositionToTradeBookingListener(PositionService<T>* _service) { service = _service; }
    ~PositionToTradeBookingListener() {} // set empty
    void ProcessAdd(Trade<T>& _data) { service->AddTrade(_data); }
    void ProcessRemove(Trade<T>& _data) {} // set empty
    void ProcessUpdate(Trade<T>& _data) {} // set empty
};

#endif
