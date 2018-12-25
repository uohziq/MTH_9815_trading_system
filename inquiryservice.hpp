/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
    Inquiry() = default; // Robert added default construct
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice() const;

  // Get the current state on the inquiry
  InquiryState GetState() const;
    void SetState(InquiryState _state) { state = _state; }
    vector<string> ToStrings() const;
private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
  inquiryId = _inquiryId;
  side = _side;
  quantity = _quantity;
  price = _price;
  state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
  return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
  return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
  return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
  return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
  return state;
}

template<typename T>
vector<string> Inquiry<T>::ToStrings() const
{
    string _inquiryId = inquiryId;
    string _product = product.GetProductId();
    string _side;
    switch (side)
    {
        case BUY:
            _side = "BUY";
            break;
        case SELL:
            _side = "SELL";
            break;
    }
    string _quantity = to_string(quantity);
    string _price = ConvertPrice(price);
    string _state;
    switch (state)
    {
        case RECEIVED:
            _state = "RECEIVED";
            break;
        case QUOTED:
            _state = "QUOTED";
            break;
        case DONE:
            _state = "DONE";
            break;
        case REJECTED:
            _state = "REJECTED";
            break;
        case CUSTOMER_REJECTED:
            _state = "CUSTOMER_REJECTED";
            break;
    }
    
    vector<string> res;
    res.push_back(_inquiryId);
    res.push_back(_product);
    res.push_back(_side);
    res.push_back(_quantity);
    res.push_back(_price);
    res.push_back(_state);
    return res;
}

// will define later
template<typename T>
class InquiryConnector;

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string, Inquiry<T> >
{
private:
    map<string, Inquiry<T> > inquiries;
    vector<ServiceListener<Inquiry<T> >*> listeners;
    InquiryConnector<T>* connector;
public:
    InquiryService();
    ~InquiryService() {} // set empty
    Inquiry<T>& GetData(string _key) { return inquiries[_key]; }
    void OnMessage(Inquiry<T>& _data);
    void AddListener(ServiceListener<Inquiry<T> >* _listener) { listeners.push_back(_listener); }
    const vector<ServiceListener<Inquiry<T> >*>& GetListeners() const { return listeners; }
    InquiryConnector<T>* GetConnector() { return connector; }
    void SendQuote(const string& _inquiryId, double _price);
    void RejectInquiry(const string& _inquiryId);
};

template<typename T>
InquiryService<T>::InquiryService()
{
    inquiries = map<string, Inquiry<T> >();
    listeners = vector<ServiceListener<Inquiry<T> >*>();
    connector = new InquiryConnector<T>(this);
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& _data)
{
    InquiryState _state = _data.GetState();
    switch (_state)
    {
        case RECEIVED:
            inquiries[_data.GetInquiryId()] = _data;
            connector->Publish(_data);
            break;
        case QUOTED:
            _data.SetState(DONE);
            inquiries[_data.GetInquiryId()] = _data;
            for (auto l = listeners.begin(); l != listeners.end(); ++l)
                (*l)->ProcessAdd(_data);
            break;
        default:
            break;
    }
}

template<typename T>
void InquiryService<T>::SendQuote(const string& _inquiryId, double _price)
{
    Inquiry<T>& _inquiry = inquiries[_inquiryId];
    InquiryState _state = _inquiry.GetState();
    _inquiry.SetPrice(_price);
    for (auto l = listeners.begin(); l != listeners.end(); ++l)
        (*l)->ProcessAdd(_inquiry);
}

template<typename T>
void InquiryService<T>::RejectInquiry(const string& _inquiryId)
{
    Inquiry<T>& _inquiry = inquiries[_inquiryId];
    _inquiry.SetState(REJECTED);
}


template<typename T>
class InquiryConnector : public Connector<Inquiry<T> >
{
    
private:
    InquiryService<T>* service;
public:
    InquiryConnector(InquiryService<T>* _service) {  service = _service; }
    ~InquiryConnector() {} // set empty
    void Publish(Inquiry<T>& _data);
    void Subscribe(ifstream& _data);
    void Subscribe(Inquiry<T>& _data) { service->OnMessage(_data); }
};

template<typename T>
void InquiryConnector<T>::Publish(Inquiry<T>& _data)
{
    InquiryState _state = _data.GetState();
    if (_state == RECEIVED)
    {
        _data.SetState(QUOTED);
        this->Subscribe(_data);
    }
}

template<typename T>
void InquiryConnector<T>::Subscribe(ifstream& _data_in)
{
    string _line;
    while (getline(_data_in, _line))
    {
        stringstream _lineStream(_line);
        string _cell;
        vector<string> _cells;
        while (getline(_lineStream, _cell, ','))
        {
            _cells.push_back(_cell);
        }
        
        string _inquiryId = _cells[0];
        string _productId = _cells[1];
        Side _side;
        if (_cells[2] == "BUY") _side = BUY;
        else if (_cells[2] == "SELL") _side = SELL;
        long _quantity = stol(_cells[3]);
        double _price = ConvertPrice(_cells[4]);
        InquiryState _state;
        if (_cells[5] == "RECEIVED") _state = RECEIVED;
        else if (_cells[5] == "QUOTED") _state = QUOTED;
        else if (_cells[5] == "DONE") _state = DONE;
        else if (_cells[5] == "REJECTED") _state = REJECTED;
        else if (_cells[5] == "CUSTOMER_REJECTED") _state = CUSTOMER_REJECTED;
        T _product = GetBond(_productId);
        Inquiry<T> _inquiry(_inquiryId, _product, _side, _quantity, _price, _state);
        service->OnMessage(_inquiry);
    }
}

#endif

