
#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <string>
#include <queue>

class NotificationManager {
public:

	static NotificationManager& GetInstance() {
		static NotificationManager notificationManager;
		return notificationManager;
	}


private:

	std::queue<std::string> notifications;

	NotificationManager() = default;
	~NotificationManager() = default;
	NotificationManager(const NotificationManager&) = delete;
	NotificationManager& operator=(const NotificationManager&) = delete;

};

#endif