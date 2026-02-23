#pragma once
#include <cassert>
#include "GeneralizedTerms/GeneralizedTermNetwork.h"
/* This class models Cost for operations in 2PC and 3PC setting
 specific Models are in separate Files or can be loaded from benchmarking files
TODO: Kosten weights aus json laden, die von extra benchmarking tool generiert wird
*/
namespace delayedresharing {

    enum BaseCosts
    {
        Addition = 10,
        Check = 15,
        Multiplication = 20,
        PRF = 400
    };
    enum DepthMode
    {
        SYMBOL, // protocol depth increases with number of symbols
        MULTIPLICATIVE // protocol depth increases with number of insequence multiplications
    };

    /*  Returns a CostPoint packlet, which tells us how the cost of this operation in the given
        semantic (Depth, Communication etc.) accounts for. The values of this CostPoint depend on
        other operations and the wider circuit context, but are not documented inside of the CostPoint
        (aside from the related Symbol).
    
    
        - For Depth Computation,    cost = how many computation rounds this computation contributes
                                           (may depend on other Operations)
        - For Computation,          cost = computation amount needed for just this operation
        - For Communication,        cost = communication needed for just this operation

        layer = only defined in case depth metric: dependency layer on which this cost point happens
    */
    struct CostPoint {
        public:
        float cost = 0.0f;
        int layer = 0;
        bool layerAssigned = false;
        delayedresharing::Symbol* relatedSymbol;
        bool operator<(const CostPoint& other) const;
    };

    class CostModel {
        public:
        DepthMode offlineDepthMode;
        DepthMode onlineDepthMode;
        CostModel()
        {

        }
         // COMMUNICATION

        virtual CostPoint OfflineCommunicationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
        {
            return {};
        };

        virtual CostPoint OnlineCommunicationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
        {
            return {};
        };

        virtual CostPoint OfflineCommunicationConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
        {
            return {};
        };

        virtual CostPoint OnlineCommunicationConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
        {
            return {};
        };

        // COMPUTATION

        virtual CostPoint OfflineComputationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
        {
            return {};
        };

        virtual CostPoint OnlineComputationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
        {
            return {};
        };

        virtual CostPoint OfflineComputationConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
        {
            return {};
        };

        virtual CostPoint OnlineComputationConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
        {
            return {};
        };
        

        // DEPTH

        virtual CostPoint OfflineDepthOperation(delayedresharing::Symbol* symbol, std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
        {
            return {};
        };

        virtual CostPoint OnlineDepthOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
        {
            return {};
        };

        virtual CostPoint OfflineDepthConversion(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
        {
            return {};
        };
        virtual CostPoint OnlineDepthConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
        {
            return {};
        };
    };




    class AnalyticalThreePCCostModel : public CostModel {
        public:
        AnalyticalThreePCCostModel()
        {
            offlineDepthMode = DepthMode::SYMBOL;
            onlineDepthMode = DepthMode::MULTIPLICATIVE;
        }
         // COMMUNICATION

        virtual CostPoint OfflineCommunicationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion);
        virtual CostPoint OnlineCommunicationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion);

        virtual CostPoint OfflineCommunicationConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode);
        virtual CostPoint OnlineCommunicationConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode);

        // COMPUTATION

        virtual CostPoint OfflineComputationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion);
        virtual CostPoint OnlineComputationOperation(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion);

        virtual CostPoint OfflineComputationConversion( delayedresharing::Symbol* symbol, std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode);
        virtual CostPoint OnlineComputationConversion( delayedresharing::Symbol* symbol, std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode);
        

        // DEPTH

        // TODO Here need to check if this should only ever be added once per circuit as it is offline phase which will be input independent single round
        virtual CostPoint OfflineDepthOperation(delayedresharing::Symbol* symbol, std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion);
        virtual CostPoint OnlineDepthOperation(delayedresharing::Symbol* symbol, std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion);

        virtual CostPoint OfflineDepthConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode);
        virtual CostPoint OnlineDepthConversion( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode);
    };
}
