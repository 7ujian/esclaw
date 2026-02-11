#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <Arduino.h>
#include <vector>

struct InboundMessage {
  String channel;
  String senderId;
  String content;
  String sessionId;
  uint32_t timestamp;
};

struct OutboundMessage {
  String channel;
  String content;
  uint32_t timestamp;
};

class MessageBus {
public:
  static MessageBus& getInstance();

  void publishInbound(const InboundMessage& msg);
  void publishOutbound(const OutboundMessage& msg);

  bool hasInbound() const;
  InboundMessage consumeInbound();

  bool hasOutbound() const;
  OutboundMessage consumeOutbound();

  void clear();

private:
  MessageBus();
  MessageBus(const MessageBus&) = delete;
  MessageBus& operator=(const MessageBus&) = delete;

  static const size_t MAX_QUEUE_SIZE = 50;

  std::vector<InboundMessage> inboundQueue_;
  std::vector<OutboundMessage> outboundQueue_;

  int inboundIndex_;
  int outboundIndex_;
};

#endif
