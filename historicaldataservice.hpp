/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP


enum ServiceType { POSITION, RISK, EXECUTION, STREAMING, INQUIRY };
// will define later
template<typename V>
class HistoricalDataConnector;
template<typename V>
class HistoricalDataListener;

/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename V>
class HistoricalDataService : Service<string, V>
{
private:
    map<string, V> historicalDatas;
    vector<ServiceListener<V>*> listeners;
    HistoricalDataConnector<V>* connector;
    ServiceListener<V>* listener;
    ServiceType type;
public:
    HistoricalDataService(); // in cast we don't know the type at initialization
    HistoricalDataService(ServiceType _type);
    ~HistoricalDataService() {} // set empty
    V& GetData(string _key) { return historicalDatas[_key]; }
    void OnMessage(V& _data) { historicalDatas[_data.GetProduct().GetProductId()] = _data; }
    void AddListener(ServiceListener<V>* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<V>*>& GetListeners() const { return listeners; }
    HistoricalDataConnector<V>* GetConnector() { return connector; }
    ServiceListener<V>* GetListener() { return listener; }
    ServiceType GetServiceType() const { return type; }
    void PersistData(string _persistKey, V& _data) { connector->Publish(_data); }
};

template<typename V>
HistoricalDataService<V>::HistoricalDataService()
{
    historicalDatas = map<string, V>();
    listeners = vector<ServiceListener<V>*>();
    connector = new HistoricalDataConnector<V>(this);
    listener = new HistoricalDataListener<V>(this);
    type = INQUIRY;
}

template<typename V>
HistoricalDataService<V>::HistoricalDataService(ServiceType _type)
{
    historicalDatas = map<string, V>();
    listeners = vector<ServiceListener<V>*>();
    connector = new HistoricalDataConnector<V>(this);
    listener = new HistoricalDataListener<V>(this);
    type = _type;
}

/**
 * Historical Data Connector publishing data from Historical Data Service.
 * Type V is the data type to persist.
 */
template<typename V>
class HistoricalDataConnector : public Connector<V>
{
private:
    HistoricalDataService<V>* service;
public:
    HistoricalDataConnector(HistoricalDataService<V>* _service) { service = _service; }
    ~HistoricalDataConnector() {} // set empty
    void Publish(V& _data);
    void Subscribe(ifstream& _data) {} // set empty
};

template<typename V>
void HistoricalDataConnector<V>::Publish(V& _data)
{
    ServiceType _type = service->GetServiceType();
    ofstream _file;
    switch (_type)
    {
        case POSITION:
            _file.open("positions.txt", ios::app);
            break;
        case RISK:
            _file.open("risk.txt", ios::app);
            break;
        case EXECUTION:
            _file.open("executions.txt", ios::app);
            break;
        case STREAMING:
            _file.open("streaming.txt", ios::app);
            break;
        case INQUIRY:
            _file.open("allinquiries.txt", ios::app);
            break;
    }
    
    _file << PrintTimeStamp() << ",";
    vector<string> _strings = _data.ToStrings();
    for (auto s = _strings.begin(); s != _strings.end(); ++s)
        _file << (*s) << ",";
    _file << endl;
}

/**
 * Historical Data Service Listener subscribing data to Historical Data.
 * Type V is the data type to persist.
 */
template<typename V>
class HistoricalDataListener : public ServiceListener<V>
{
private:
    HistoricalDataService<V>* service;
public:
    HistoricalDataListener(HistoricalDataService<V>* _service) { service = _service; }
    ~HistoricalDataListener() {} // set empty
    void ProcessAdd(V& _data);
    void ProcessRemove(V& _data) {} // set empty
    void ProcessUpdate(V& _data) {} // set empty
};

template<typename V>
void HistoricalDataListener<V>::ProcessAdd(V& _data)
{
    string _persistKey = _data.GetProduct().GetProductId();
    service->PersistData(_persistKey, _data);
}

#endif
