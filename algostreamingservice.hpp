//
//  algostreamingservice.hpp
//  tradingsystem
//
//  Created by Robert on 12/22/18.
//  Copyright © 2018 Zhou Robert Qi. All rights reserved.
//

#ifndef algostreamingservice_hpp
#define algostreamingservice_hpp

#include <string>
#include "soa.hpp"
#include "pricingservice.hpp"

enum PricingSide { BID, OFFER };

class PriceStreamOrder
{
public:
    PriceStreamOrder() = default;
    PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);
    double GetPrice() const {return price;}
    long GetVisibleQuantity() const {return visibleQuantity;}
    long GetHiddenQuantity() const {return hiddenQuantity;}
    PricingSide GetSide() const {return side;}
    vector<string> ToStrings() const;
private:
    double price;
    long visibleQuantity;
    long hiddenQuantity;
    PricingSide side;
};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
    price = _price;
    visibleQuantity = _visibleQuantity;
    hiddenQuantity = _hiddenQuantity;
    side = _side;
}

vector<string> PriceStreamOrder::ToStrings() const
{
    string _price = ConvertPrice(price);
    string _visibleQuantity = to_string(visibleQuantity);
    string _hiddenQuantity = to_string(hiddenQuantity);
    string _side;
    switch (side)
    {
        case BID:
            _side = "BID";
            break;
        case OFFER:
            _side = "OFFER";
            break;
    }
    
    vector<string> _strings;
    _strings.push_back(_price);
    _strings.push_back(_visibleQuantity);
    _strings.push_back(_hiddenQuantity);
    _strings.push_back(_side);
    return _strings;
}


template<typename T>
class PriceStream
{
public:
    PriceStream() = default;
    PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);
    const T& GetProduct() const {return product;}
    const PriceStreamOrder& GetBidOrder() const {return bidOrder;}
    const PriceStreamOrder& GetOfferOrder() const {return offerOrder;}
    vector<string> ToStrings() const;
private:
    T product;
    PriceStreamOrder bidOrder;
    PriceStreamOrder offerOrder;
};

template<typename T>
PriceStream<T>::PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder) :
product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
vector<string> PriceStream<T>::ToStrings() const
{
    string _product = product.GetProductId();
    vector<string> _bidOrder = bidOrder.ToStrings();
    vector<string> _offerOrder = offerOrder.ToStrings();
    
    vector<string> _strings;
    _strings.push_back(_product);
    _strings.insert(_strings.end(), _bidOrder.begin(), _bidOrder.end());
    _strings.insert(_strings.end(), _offerOrder.begin(), _offerOrder.end());
    return _strings;
}

template<typename T>
class AlgoStream
{
public:
    AlgoStream() = default;
    AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);
    PriceStream<T>* GetPriceStream() const { return priceStream; }
private:
    PriceStream<T>* priceStream;
};

template<typename T>
AlgoStream<T>::AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
{
    priceStream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
}


// will define later
template<typename T>
class AlgoStreamingToPricingListener;


template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T> >
{
private:
    map<string, AlgoStream<T> > algoStreams;
    vector<ServiceListener<AlgoStream<T> >*> listeners;
    ServiceListener<Price<T> >* listener;
    long count;
public:
    AlgoStreamingService();
    ~AlgoStreamingService() {} // set empty
    AlgoStream<T>& GetData(string _key) { return algoStreams[_key]; }
    void OnMessage(AlgoStream<T>& _data);
    void AddListener(ServiceListener<AlgoStream<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<AlgoStream<T> >*>& GetListeners() const { return listeners; }
    ServiceListener<Price<T> >* GetListener() { return listener; }
    void AlgoPublishPrice(Price<T>& _price);
};

template<typename T>
AlgoStreamingService<T>::AlgoStreamingService()
{
    algoStreams = map<string, AlgoStream<T> >();
    listeners = vector<ServiceListener<AlgoStream<T> >*>();
    listener = new AlgoStreamingToPricingListener<T>(this);
    count = 0;
}


template<typename T>
void AlgoStreamingService<T>::OnMessage(AlgoStream<T>& _data)
{
    algoStreams[_data.GetPriceStream()->GetProduct().GetProductId()] = _data;
}

template<typename T>
void AlgoStreamingService<T>::AlgoPublishPrice(Price<T>& _price)
{
    T _product = _price.GetProduct();
    string _productId = _product.GetProductId();
    
    double _mid = _price.GetMid();
    double _bidOfferSpread = _price.GetBidOfferSpread();
    double _bidPrice = _mid - _bidOfferSpread / 2.0;
    double _offerPrice = _mid + _bidOfferSpread / 2.0;
    long _visibleQuantity = (count % 2 + 1) * 10000000;
    long _hiddenQuantity = _visibleQuantity * 2;
    
    count++;
    PriceStreamOrder _bidOrder(_bidPrice, _visibleQuantity, _hiddenQuantity, BID);
    PriceStreamOrder _offerOrder(_offerPrice, _visibleQuantity, _hiddenQuantity, OFFER);
    AlgoStream<T> _algoStream(_product, _bidOrder, _offerOrder);
    algoStreams[_productId] = _algoStream;
    
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_algoStream);
}


template<typename T>
class AlgoStreamingToPricingListener : public ServiceListener<Price<T> >
{
private:
    AlgoStreamingService<T>* service;
public:
    AlgoStreamingToPricingListener(AlgoStreamingService<T>* _service) { service = _service; }
    ~AlgoStreamingToPricingListener() {} // set empty
    void ProcessAdd(Price<T>& _data) { service->AlgoPublishPrice(_data); }
    void ProcessRemove(Price<T>& _data) {} // set empty
    void ProcessUpdate(Price<T>& _data) {} // set empty
};

#endif /* algostreamingservice_hpp */
