/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham, Robert
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include<boost/algorithm/string.hpp>

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{
public:
    // ctor for a price
    Price() = default;
    Price(const T& _product, double _mid, double _bidOfferSpread);
    
    // Get the product
    const T& GetProduct() const;
    // Get the mid price
    double GetMid() const;
    // Get the bid/offer spread around the mid
    double GetBidOfferSpread() const;
    vector<string> print();
private:
    T product;
    double mid;
    double bidOfferSpread;
};

template<typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread) :
product(_product)
{
    mid = _mid;
    bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
    return product;
}

template<typename T>
double Price<T>::GetMid() const
{
    return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
    return bidOfferSpread;
}

template<typename T>
vector<string> Price<T>::print()
{
    string _product = product.GetProductId();
    string _mid = ConvertPrice(mid);
    string _bidOfferSpread = ConvertPrice(bidOfferSpread);
    
    vector<string> res;
    res.push_back(_product);
    res.push_back(_mid);
    res.push_back(_bidOfferSpread);
    return res;
}


template<typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{
private:
    map<string, Price<T> > prices;
    vector<ServiceListener<Price<T> >*> listeners;
    PricingConnector<T>* connector;
public:
    PricingService()
    {
        prices = map<string, Price<T> >();
        listeners = vector<ServiceListener<Price<T> >*>();
        connector = new PricingConnector<T>(this);
    }
    Price<T>& GetData(string _key)
    {
        return prices[_key];
    }
    void OnMessage(Price<T>& _data) // send data to other services, listener are created by other serices and registered to this pricing service
    {
        prices[_data.GetProduct().GetProductId()] = _data;
        for(auto l = listeners.begin(); l !=listeners.end(); ++l)
            (*l)->ProcessAdd(_data);
    }
    void AddListener(ServiceListener<Price<T> >* _listener)
    {
        listeners.push_back(_listener);
    }
    const vector<ServiceListener<Price<T> >*>& GetListeners() const
    {
        return listeners;
    }
    PricingConnector<T>* GetConnector()
    {
        return connector;
    }
};



// connector to Pricing Service
template<typename T>
class PricingConnector : public Connector<Price<T> >
{
private:
    PricingService<T>* service;
public:
    // Connector and Destructor
    PricingConnector(PricingService<T>* _service)
    {
        service = _service;
    }
    ~PricingConnector() {} // set empty
    // Publish data to the Connector
    void Publish(Price<T>& _data) {} // set empty
    // Subscribe data from the Connector
    void Subscribe(ifstream& _data_in);
};

template<typename T>
void PricingConnector<T>::Subscribe(ifstream& _data_in)
{
    string _thisline;
    while (getline(_data_in, _thisline))
    {
        stringstream _lineStream(_thisline);
        string _item;
        vector<string> _item_parsing;
        while (getline(_lineStream, _item, ','))
        {
            boost::algorithm::trim(_item);
            _item_parsing.push_back(_item);
        }
        
        string _productId = _item_parsing[0];
        double _bidPrice = ConvertPrice(_item_parsing[1]);
        double _offerPrice = ConvertPrice(_item_parsing[2]);
        double _midPrice = (_bidPrice + _offerPrice) / 2.;
        double _spread = _offerPrice - _bidPrice;
        T _product = GetBond(_productId);
        Price<T> _price(_product, _midPrice, _spread);
        service->OnMessage(_price);
    }
}

#endif
