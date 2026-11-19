#ifndef ELKY5_COMMON_SINGLETON_H_
#define ELKY5_COMMON_SINGLETON_H_

namespace Common {

template <class T>
class Singleton {
public:
	// Created on first use and never destroyed.
	static T* Instance() {
		if (g_instance == nullptr) {
			g_instance = new T;
		}
		return g_instance;
	}

private:
	static inline T* g_instance = nullptr;
};

} // namespace Common

#endif /* ELKY5_COMMON_SINGLETON_H_ */
