#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "llm_provider.h"

#define MAX_SESSIONS 5
#define MAX_MESSAGES_PER_SESSION 20
#define MAX_CONTENT_LENGTH 512
#define SESSION_ID_LENGTH 12

struct SessionMessage {
  String role;
  String content;
};

struct Session {
  String sessionId;
  SessionMessage messages[MAX_MESSAGES_PER_SESSION];
  int messageCount;
  unsigned long lastUpdated;
};

class SessionManager {
public:
  static SessionManager& getInstance();

  Session* createSession(const String& channelId, const String& senderId);
  Session* getSession(const String& sessionId);
  Session* getOrCreateSession(const String& channelId, const String& senderId);
  
  bool addMessage(const String& sessionId, const String& role, const String& content);
  bool updateLastAccess(const String& sessionId);
  
  void saveSession(const String& sessionId);
  void saveAllSessions();
  bool loadSession(const String& sessionId);
  void loadAllSessions();
  
  void cleanupOldSessions();
  void deleteSession(const String& sessionId);
  void clearAll();
  
  int getSessionCount() const;

private:
  SessionManager();
  SessionManager(const SessionManager&) = delete;
  SessionManager& operator=(const SessionManager&) = delete;

  String generateSessionId();
  String getSessionPath(const String& sessionId);

  Session sessions_[MAX_SESSIONS];
  int sessionCount_;
};

#endif
