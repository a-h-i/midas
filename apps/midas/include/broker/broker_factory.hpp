#pragma once
#include "broker-interface/broker.hpp"
#include <memory>

std::shared_ptr<midas::Broker> createIBKRBroker();