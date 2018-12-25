/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
// #include "marketdataservice.hpp"
#include "algostreamingservice.hpp"

// Robert moved class PriceStreamOrder and PriceStream to algostreamingservice.hpp

// will define later
template<typename T>
class StreamingToAlgoStreamingListener;


template<typename T>
class StreamingService : public Service<string, PriceStream<T> >
{
private:
    map<string, PriceStream<T> > priceStreams;
    vector<ServiceListener<PriceStream<T> >*> listeners;
    ServiceListener<AlgoStream<T> >* listener;
public:
    StreamingService();
    ~StreamingService() {} // set empty
    PriceStream<T>& GetData(string _key) { return priceStreams[_key]; }
    void OnMessage(PriceStream<T>& _data) { priceStreams[_data.GetProduct().GetProductId()] = _data; }
    void AddListener(ServiceListener<PriceStream<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<PriceStream<T> >*>& GetListeners() const { return listeners; }
    ServiceListener<AlgoStream<T> >* GetListener() { return listener; }
    void PublishPrice(PriceStream<T>& _priceStream);
};

template<typename T>
StreamingService<T>::StreamingService()
{
    priceStreams = map<string, PriceStream<T> >();
    listeners = vector<ServiceListener<PriceStream<T> >*>();
    listener = new StreamingToAlgoStreamingListener<T>(this);
}

template<typename T>
void StreamingService<T>::PublishPrice(PriceStream<T>& _priceStream)
{
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_priceStream);
}


template<typename T>
class StreamingToAlgoStreamingListener : public ServiceListener<AlgoStream<T> >
{
private:
    StreamingService<T>* service;
public:
    StreamingToAlgoStreamingListener(StreamingService<T>* _service) { service = _service; }
    ~StreamingToAlgoStreamingListener() {} // set empty
    void ProcessAdd(AlgoStream<T>& _data);
    void ProcessRemove(AlgoStream<T>& _data) {} // set empty
    void ProcessUpdate(AlgoStream<T>& _data) {} // set empty
};


template<typename T>
void StreamingToAlgoStreamingListener<T>::ProcessAdd(AlgoStream<T>& _data)
{
    PriceStream<T>* _priceStream = _data.GetPriceStream();
    service->OnMessage(*_priceStream);
    service->PublishPrice(*_priceStream);
}

#endif
