#include "message_bus.h"

MessageBus::MessageBus() : inboundIndex_(0), outboundIndex_(0) {
  inboundQueue_.reserve(MAX_QUEUE_SIZE);
  outboundQueue_.reserve(MAX_QUEUE_SIZE);
}

MessageBus& MessageBus::getInstance() {
  static MessageBus instance;
  return instance;
}

void MessageBus::publishInbound(const InboundMessage& msg) {
  if (inboundQueue_.size() >= MAX_QUEUE_SIZE) {
    inboundQueue_.erase(inboundQueue_.begin());
  }

  inboundQueue_.push_back(msg);
}

void MessageBus::publishOutbound(const OutboundMessage& msg) {
  if (outboundQueue_.size() >= MAX_QUEUE_SIZE) {
    outboundQueue_.erase(outboundQueue_.begin());
  }

  outboundQueue_.push_back(msg);
}

bool MessageBus::hasInbound() const {
  return inboundIndex_ < inboundQueue_.size();
}

InboundMessage MessageBus::consumeInbound() {
  if (!hasInbound()) {
    return InboundMessage();
  }

  InboundMessage msg = inboundQueue_[inboundIndex_];
  inboundIndex_++;

  if (inboundIndex_ >= inboundQueue_.size()) {
    inboundQueue_.clear();
    inboundIndex_ = 0;
  }

  return msg;
}

bool MessageBus::hasOutbound() const {
  return outboundIndex_ < outboundQueue_.size();
}

OutboundMessage MessageBus::consumeOutbound() {
  if (!hasOutbound()) {
    return OutboundMessage();
  }

  OutboundMessage msg = outboundQueue_[outboundIndex_];
  outboundIndex_++;

  if (outboundIndex_ >= outboundQueue_.size()) {
    outboundQueue_.clear();
    outboundIndex_ = 0;
  }

  return msg;
}

void MessageBus::clear() {
  inboundQueue_.clear();
  outboundQueue_.clear();
  inboundIndex_ = 0;
  outboundIndex_ = 0;
}
