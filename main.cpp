//
//  main.cpp
//  tradingsystem
//
//  Created by Robert on 12/21/18.
//  Copyright © 2018 Zhou Robert Qi. All rights reserved.
//

#include <iostream>
#include <string>
#include <map>
#include <fstream>

using namespace std;
#include <stdio.h>
#include "products.hpp"
#include "tools.hpp"
#include "soa.hpp"

// lane 1
#include "pricingservice.hpp"
#include "guiservice.hpp"
#include "algostreamingservice.hpp"
#include "streamingservice.hpp"
// lane 2
#include "marketdataservice.hpp"
#include "algoexecutionservice.hpp"
#include "executionservice.hpp"
// lane 3
#include "tradebookingservice.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"
// lane 4
#include "inquiryservice.hpp"
// lane combination
#include "historicaldataservice.hpp"


int main(int argc, const char * argv[])
{
    // initialize all the services
    // lane 1
    cout << PrintTimeStamp() << " start to initialize all the services" << endl;
    PricingService<Bond> pricingService;
    GUIService<Bond> guiService;
    AlgoStreamingService<Bond> algoStreamingService;
    StreamingService<Bond> streamingService;
    // lane 2
    MarketDataService<Bond> marketDataService;
    AlgoExecutionService<Bond> algoExecutionService;
    ExecutionService<Bond> executionService;
    // lane 3
    TradeBookingService<Bond> tradeBookingService;
    PositionService<Bond> positionService;
    RiskService<Bond> riskService;
    // lane 4
    InquiryService<Bond> inquiryService;
    // lane combination
    HistoricalDataService<PriceStream<Bond> > historicalStreamingService(STREAMING);
    HistoricalDataService<ExecutionOrder<Bond> > historicalExecutionService(EXECUTION);
    HistoricalDataService<Position<Bond> > historicalPositionService(POSITION);
    HistoricalDataService<PV01<Bond> > historicalRiskService(RISK);
    HistoricalDataService<Inquiry<Bond> > historicalInquiryService(INQUIRY);
    cout << PrintTimeStamp() << " finished!" << endl;
    
    // link all the services
    cout << PrintTimeStamp() << " start to link all the services" << endl;
    // lane 1
    pricingService.AddListener(algoStreamingService.GetListener());
    pricingService.AddListener(guiService.GetListener());
    algoStreamingService.AddListener(streamingService.GetListener());
    // lane 2
    marketDataService.AddListener(algoExecutionService.GetListener());
    algoExecutionService.AddListener(executionService.GetListener());
    // lane 3
    executionService.AddListener(tradeBookingService.GetListener()); // cross lane
    tradeBookingService.AddListener(positionService.GetListener());
    positionService.AddListener(riskService.GetListener());
    // lane 4
    ;
    // lane combination
    streamingService.AddListener(historicalStreamingService.GetListener());
    executionService.AddListener(historicalExecutionService.GetListener());
    positionService.AddListener(historicalPositionService.GetListener());
    riskService.AddListener(historicalRiskService.GetListener());
    inquiryService.AddListener(historicalInquiryService.GetListener());
    cout << PrintTimeStamp() << " finished!" << endl;
    
    // process data
    cout << PrintTimeStamp() << " start to process input data" << endl;
    // lane 1
    ifstream priceData("prices.txt");
    pricingService.GetConnector()->Subscribe(priceData);
    // lane 2
    ifstream marketData("marketdata.txt");
    marketDataService.GetConnector()->Subscribe(marketData);
    // lane 3
    ifstream tradeData("trades.txt");
    tradeBookingService.GetConnector()->Subscribe(tradeData);
    // lane 4
    ifstream inquiryData("inquiries.txt");
    inquiryService.GetConnector()->Subscribe(inquiryData);
    cout << PrintTimeStamp() << " finished" << endl;
    
    // insert code here...
    cout << PrintTimeStamp() << endl << "system finished!\n";
    
    cout << "press \"Enter\" key to exit" << endl;
    getchar();
    
    return 0;
}
