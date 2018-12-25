//
//  guiservice.hpp
//  tradingsystem
//
//  Created by Robert on 12/22/18.
//  Copyright © 2018 Zhou Robert Qi. All rights reserved.
//

#ifndef guiservice_hpp
#define guiservice_hpp

#include <iostream>
#include "soa.hpp"
#include "pricingservice.hpp"

template<typename T>
class GUIConnector; // output to txt file
template<typename T>
class GUIToPricingListener; // get data automatically from pricing service


template<typename T>
class GUIService : Service<string, Price<T> >
{
private:
    map<string, Price<T> > guis;
    vector<ServiceListener<Price<T> >*> listeners;
    GUIConnector<T>* connector;
    ServiceListener<Price<T> >* listener;
    int throttle;
    long millisec;
public:
    // Constructor and destructor
    GUIService();
    ~GUIService() {} // set empty
    Price<T>& GetData(string _key) {return guis[_key];}
    void OnMessage(Price<T>& _data);
    void AddListener(ServiceListener<Price<T> >* _listener) {listeners.push_back(_listener);}
    const vector<ServiceListener<Price<T> >*>& GetListeners() const {return listeners;}
    GUIConnector<T>* GetConnector() {return connector;}
    ServiceListener<Price<T> >* GetListener() {return listener;}
    int GetThrottle() const {return throttle;}
    long GetMillisec() const {return millisec;}
    void SetMillisec(long _millisec) {millisec = _millisec;}
};

template<typename T>
GUIService<T>::GUIService()
{
    guis = map<string, Price<T> >();
    listeners = vector<ServiceListener<Price<T> >*>();
    connector = new GUIConnector<T>(this);
    listener = new GUIToPricingListener<T>(this);
    throttle = 300;
    millisec = 0;
}

template<typename T>
void GUIService<T>::OnMessage(Price<T>& _data)
{
    guis[_data.GetProduct().GetProductId()] = _data;
    connector->Publish(_data);
}

template<typename T>
class GUIConnector : public Connector<Price<T> >
{
private:
    GUIService<T>* service;
public:
    GUIConnector(GUIService<T>* _service) { service = _service;}
    ~GUIConnector() {}
    void Publish(Price<T>& _data); // output to txt file
    void Subscribe(ifstream& _data) {} // set empty
};

template<typename T>
void GUIConnector<T>::Publish(Price<T>& _data_out)
{
    int _throttle = service->GetThrottle();
    long _millisec = service->GetMillisec();
    long _millisecNow = GetMillisecond();
    while (_millisecNow < _millisec) _millisecNow += 1000;
    if (_millisecNow - _millisec >= _throttle)
    {
        service->SetMillisec(_millisecNow);
        ofstream _file;
        _file.open("gui.txt", ios::app);
        
        _file << PrintTimeStamp() << ",";
        vector<string> _strings = _data_out.print();
        for (auto s = _strings.begin(); s != _strings.end(); ++s)
            _file << *s << ",";
        _file << endl;
    }
}


template<typename T>
class GUIToPricingListener : public ServiceListener<Price<T> >
{
private:
    GUIService<T>* service;
public:
    GUIToPricingListener(GUIService<T>* _service) {service = _service;}
    ~GUIToPricingListener() {} // set empty
    void ProcessAdd(Price<T>& _data) {service->OnMessage(_data);}
    void ProcessRemove(Price<T>& _data) {} // set empty
    void ProcessUpdate(Price<T>& _data) {} // set empty
};

#endif /* guiservice_hpp */
