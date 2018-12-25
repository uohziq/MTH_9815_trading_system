/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
    PV01() = default;
  PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
    const T& GetProduct() const { return product; }

  // Get the PV01 value
    double GetPV01() const { return pv01; }

  // Get the quantity that this risk value is associated with
    long GetQuantity() const { return quantity; }
    void SetQuantity(long _quantity) { quantity = _quantity; }
    vector<string> ToStrings() const;
private:
  T product;
  double pv01;
  long quantity;

};

template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
product(_product)
{
    pv01 = _pv01;
    quantity = _quantity;
}

template<typename T>
vector<string> PV01<T>::ToStrings() const
{
    string _product = product.GetProductId();
    string _pv01 = to_string(pv01);
    string _quantity = to_string(quantity);
    
    vector<string> res;
    res.push_back(_product);
    res.push_back(_pv01);
    res.push_back(_quantity);
    return res;
}

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
    BucketedSector() = default;
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};



template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products)
{
  name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
  return name;
}


// will define later
template<typename T>
class RiskToPositionListener;


template<typename T>
class RiskService : public Service<string, PV01<T> >
{
private:
    map<string, PV01<T> > pv01s;
    vector<ServiceListener<PV01<T> >*> listeners;
    RiskToPositionListener<T>* listener;
public:
    RiskService();
    ~RiskService() {} // set empty
    PV01<T>& GetData(string _key) { return pv01s[_key]; }
    void OnMessage(PV01<T>& _data) { pv01s[_data.GetProduct().GetProductId()] = _data; }
    void AddListener(ServiceListener<PV01<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<PV01<T> >*>& GetListeners() const { return listeners; }
    RiskToPositionListener<T>* GetListener() { return listener; }
    void AddPosition(Position<T>& _position);
    const PV01<BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T>& _sector) const;
};

template<typename T>
RiskService<T>::RiskService()
{
    pv01s = map<string, PV01<T> >();
    listeners = vector<ServiceListener<PV01<T> >*>();
    listener = new RiskToPositionListener<T>(this);
}

template<typename T>
void RiskService<T>::AddPosition(Position<T>& _position)
{
    T _product = _position.GetProduct();
    string _productId = _product.GetProductId();
    double _pv01Value = GetPV01Value(_productId);
    long _quantity = _position.GetAggregatePosition();
    PV01<T> _pv01(_product, _pv01Value, _quantity);
    pv01s[_productId] = _pv01;
    
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_pv01);
}

template<typename T>
const PV01<BucketedSector<T> >& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& _sector) const
{
    BucketedSector<T> _product = _sector;
    double _pv01 = 0;
    long _quantity = 1;
    
    vector<T>& _products = _sector.GetProducts();
    for (auto p = _products.begin(); p != _products.end(); ++p)
    {
        string _pId = p->GetProductId();
        _pv01 += pv01s[_pId].GetPV01() * pv01s[_pId].GetQuantity();
    }
    
    return PV01<BucketedSector<T> >(_product, _pv01, _quantity);
}

/**
 * Risk Service Listener subscribing data from Position Service to Risk Service.
 * Type T is the product type.
 */
template<typename T>
class RiskToPositionListener : public ServiceListener<Position<T> >
{
private:
    RiskService<T>* service;
public:
    RiskToPositionListener(RiskService<T>* _service) { service = _service; }
    ~RiskToPositionListener() {} // set empty
    void ProcessAdd(Position<T>& _data) { service->AddPosition(_data); }
    void ProcessRemove(Position<T>& _data)  {} // set empty
    void ProcessUpdate(Position<T>& _data) {} // set empty
};

#endif
