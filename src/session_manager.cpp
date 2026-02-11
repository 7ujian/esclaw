#include "session_manager.h"
#include <LittleFS.h>

SessionManager::SessionManager()
  : sessionCount_(0) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    sessions_[i].sessionId = "";
    sessions_[i].messageCount = 0;
    sessions_[i].lastUpdated = 0;
  }
}

SessionManager& SessionManager::getInstance() {
  static SessionManager instance;
  return instance;
}

String SessionManager::generateSessionId() {
  const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  String id = "";
  for (int i = 0; i < SESSION_ID_LENGTH; i++) {
    id += chars[random(0, 35)];
  }
  return id;
}

String SessionManager::getSessionPath(const String& sessionId) {
  return "/sessions/" + sessionId + ".json";
}

Session* SessionManager::createSession(const String& channelId, const String& senderId) {
  if (sessionCount_ >= MAX_SESSIONS) {
    cleanupOldSessions();
  }

  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions_[i].sessionId.isEmpty()) {
      sessions_[i].sessionId = generateSessionId();
      sessions_[i].messageCount = 0;
      sessions_[i].lastUpdated = millis();
      sessionCount_++;
      return &sessions_[i];
    }
  }
  return nullptr;
}

Session* SessionManager::getSession(const String& sessionId) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions_[i].sessionId == sessionId) {
      return &sessions_[i];
    }
  }
  return nullptr;
}

Session* SessionManager::getOrCreateSession(const String& channelId, const String& senderId) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions_[i].sessionId.isEmpty()) {
      Session* session = createSession(channelId, senderId);
      return session;
    }
  }
  
  Session* oldest = &sessions_[0];
  for (int i = 1; i < MAX_SESSIONS; i++) {
    if (sessions_[i].lastUpdated < oldest->lastUpdated) {
      oldest = &sessions_[i];
    }
  }
  
  oldest->sessionId = generateSessionId();
  oldest->messageCount = 0;
  oldest->lastUpdated = millis();
  return oldest;
}

bool SessionManager::addMessage(const String& sessionId, const String& role, const String& content) {
  Session* session = getSession(sessionId);
  if (!session) {
    return false;
  }

  if (session->messageCount >= MAX_MESSAGES_PER_SESSION) {
    for (int i = 0; i < session->messageCount - 1; i++) {
      sessions_[i].messages[i] = session->messages[i + 1];
    }
    session->messageCount--;
  }

  int idx = session->messageCount;
  sessions_[idx].messages[idx].role = role;
  sessions_[idx].messages[idx].content = content.substring(0, MAX_CONTENT_LENGTH);
  session->messageCount++;
  session->lastUpdated = millis();

  return true;
}

bool SessionManager::updateLastAccess(const String& sessionId) {
  Session* session = getSession(sessionId);
  if (!session) {
    return false;
  }
  session->lastUpdated = millis();
  return true;
}

void SessionManager::saveSession(const String& sessionId) {
  Session* session = getSession(sessionId);
  if (!session) {
    return;
  }

  StaticJsonDocument<1024> doc;
  doc["sessionId"] = session->sessionId;
  doc["lastUpdated"] = session->lastUpdated;

  JsonArray messages = doc.createNestedArray("messages");
  for (int i = 0; i < session->messageCount && i < 10; i++) {
    JsonObject msg = messages.createNestedObject();
    msg["role"] = session->messages[i].role;
    msg["content"] = session->messages[i].content;
  }

  String path = getSessionPath(sessionId);
  LittleFS.mkdir("/sessions");

  File file = LittleFS.open(path, "w");
  if (!file) {
    return;
  }

  serializeJson(doc, file);
  file.close();
}

void SessionManager::saveAllSessions() {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (!sessions_[i].sessionId.isEmpty()) {
      saveSession(sessions_[i].sessionId);
    }
  }
}

bool SessionManager::loadSession(const String& sessionId) {
  String path = getSessionPath(sessionId);
  File file = LittleFS.open(path, "r");
  if (!file) {
    return false;
  }

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    return false;
  }

  if (sessionCount_ >= MAX_SESSIONS) {
    cleanupOldSessions();
  }

  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions_[i].sessionId.isEmpty()) {
      sessions_[i].sessionId = doc["sessionId"] | "";
      sessions_[i].lastUpdated = doc["lastUpdated"] | 0;
      sessions_[i].messageCount = 0;

      JsonArray messages = doc["messages"].as<JsonArray>();
      for (JsonVariant msg : messages) {
        if (sessions_[i].messageCount < MAX_MESSAGES_PER_SESSION) {
          sessions_[i].messages[sessions_[i].messageCount].role = msg["role"] | "";
          sessions_[i].messages[sessions_[i].messageCount].content = msg["content"] | "";
          sessions_[i].messageCount++;
        }
      }
      sessionCount_++;
      return true;
    }
  }
  return false;
}

void SessionManager::loadAllSessions() {
  LittleFS.mkdir("/sessions");

  File root = LittleFS.open("/sessions");
  if (!root || !root.isDirectory()) {
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    if (fileName.endsWith(".json")) {
      int start = fileName.indexOf("/", 10);
      if (start == -1) start = 0;
      int end = fileName.lastIndexOf(".json");
      String sessionId = fileName.substring(start + 1, end);
      loadSession(sessionId);
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
}

void SessionManager::cleanupOldSessions() {
  const unsigned long MAX_AGE_MS = 24 * 60 * 60 * 1000;
  unsigned long now = millis();

  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (!sessions_[i].sessionId.isEmpty()) {
      if (now - sessions_[i].lastUpdated > MAX_AGE_MS) {
        deleteSession(sessions_[i].sessionId);
      }
    }
  }
}

void SessionManager::deleteSession(const String& sessionId) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions_[i].sessionId == sessionId) {
      sessions_[i].sessionId = "";
      sessions_[i].messageCount = 0;
      sessions_[i].lastUpdated = 0;
      sessionCount_--;

      String path = getSessionPath(sessionId);
      LittleFS.remove(path);
      return;
    }
  }
}

void SessionManager::clearAll() {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (!sessions_[i].sessionId.isEmpty()) {
      String path = getSessionPath(sessions_[i].sessionId);
      LittleFS.remove(path);
    }
    sessions_[i].sessionId = "";
    sessions_[i].messageCount = 0;
    sessions_[i].lastUpdated = 0;
  }
  sessionCount_ = 0;
}

int SessionManager::getSessionCount() const {
  return sessionCount_;
}
