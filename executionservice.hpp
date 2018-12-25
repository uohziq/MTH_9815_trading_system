/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

// #include <string>
// #include "soa.hpp"
// #include "marketdataservice.hpp"

// enum OrderType, enum Market, class ExecutionOrder, are defined in the algoexecutionservice.hpp
/*
enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };
*/
/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
/*
template<typename T>
class ExecutionOrder
{

public:

  // ctor for an order
  ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the order ID
  const string& GetOrderId() const;

  // Get the order type on this order
  OrderType GetOrderType() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;

};



template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
  product(_product)
{
  side = _side;
  orderId = _orderId;
  orderType = _orderType;
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  parentOrderId = _parentOrderId;
  isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
  return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
  return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
  return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
  return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
  return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
  return isChildOrder;
}

*/


// will define later
template<typename T>
class ExecutionToAlgoExecutionListener;

// changed from pure virtual function into normal virtual function
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder<T> >
{
private:
    map<string, ExecutionOrder<T> > executionOrders;
    vector<ServiceListener<ExecutionOrder<T> >* > listeners;
    ExecutionToAlgoExecutionListener<T>* listener;
public:
    ExecutionService();
    ~ExecutionService() {} // set empty
    ExecutionOrder<T>& GetData(string _key) { return executionOrders[_key]; }
    void OnMessage(ExecutionOrder<T>& _data) { executionOrders[_data.GetProduct().GetProductId()] = _data;}
    void AddListener(ServiceListener<ExecutionOrder<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<ExecutionOrder<T> >* >& GetListeners() const { return listeners; }
    ExecutionToAlgoExecutionListener<T>* GetListener() { return listener; }
    void ExecuteOrder(ExecutionOrder<T>& _executionOrder);
};

template<typename T>
ExecutionService<T>::ExecutionService()
{
    executionOrders = map<string, ExecutionOrder<T> >();
    listeners = vector<ServiceListener<ExecutionOrder<T> >*>();
    listener = new ExecutionToAlgoExecutionListener<T>(this);
}

template<typename T>
void ExecutionService<T>::ExecuteOrder(ExecutionOrder<T>& _executionOrder)
{
    string _productId = _executionOrder.GetProduct().GetProductId();
    executionOrders[_productId] = _executionOrder;
    
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_executionOrder);
}

/**
 * Execution Service Listener subscribing data from Algo Execution Service to Execution Service.
 * Type T is the product type.
 */
template<typename T>
class ExecutionToAlgoExecutionListener : public ServiceListener<AlgoExecution<T> >
{
private:
    ExecutionService<T>* service;
public:
    ExecutionToAlgoExecutionListener(ExecutionService<T>* _service) { service = _service; }
    ~ExecutionToAlgoExecutionListener() {} // set empty
    void ProcessAdd(AlgoExecution<T>& _data);
    void ProcessRemove(AlgoExecution<T>& _data) {} // set empty
    void ProcessUpdate(AlgoExecution<T>& _data) {} // set empty
};

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessAdd(AlgoExecution<T>& _data)
{
    ExecutionOrder<T>* _executionOrder = _data.GetExecutionOrder();
    service->OnMessage(*_executionOrder);
    service->ExecuteOrder(*_executionOrder);
}

#endif
