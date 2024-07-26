#include <jni.h>
#include <string>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <android/log.h>

#define LOG_TAG "IP_LOGGER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Function to check if an IPv6 address is global unicast
bool isGlobalUnicastIPv6(struct sockaddr_in6 *addr) {
    return (addr->sin6_addr.s6_addr[0] & 0xE0) == 0x20;
}

// Function to check if an IPv4 address is public
bool isPublicIPv4(struct sockaddr_in *addr) {
    uint32_t ip = ntohl(addr->sin_addr.s_addr);

    // Check for private IPv4 addresses, link-local addresses, CGNAT, and loopback addresses
    if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) ||  // 10.0.0.0/8
        (ip >= 0xAC100000 && ip <= 0xAC1FFFFF) ||  // 172.16.0.0/12
        (ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) ||  // 192.168.0.0/16
        (ip >= 0xA9FE0000 && ip <= 0xA9FEFFFF) ||  // 169.254.0.0/16 (link local)
        (ip >= 0x64400000 && ip <= 0x647FFFFF) ||  // 100.64.0.0/10 (CGNAT)
        (ip >= 0x7F000000 && ip <= 0x7FFFFFFF)) {  // 127.0.0.0/8 (loopback)
        return false;
    }

    return true;
}

// Function to check if an IPv4 address is private
bool isPrivateIPv4(struct sockaddr_in *addr) {
    uint32_t ip = ntohl(addr->sin_addr.s_addr);

    // Check for private IPv4 addresses
    if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) ||  // 10.0.0.0/8
        (ip >= 0xAC100000 && ip <= 0xAC1FFFFF) ||  // 172.16.0.0/12
        (ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) ||  // 192.168.0.0/16
        (ip >= 0xA9FE0000 && ip <= 0xA9FEFFFF)) {  // 169.254.0.0/16 (link local)
        return true;
    }

    return false;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_jamesmobiledev_cloudonixtask_MainActivity_getIPAddress(JNIEnv *env, jobject thiz) {
    std::string ipAddress = "Unable to get IP address";

    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    std::string fallbackIPv4;

    if (getifaddrs(&ifaddr) == -1) {
        return env->NewStringUTF(ipAddress.c_str());
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET6) {
            // Check for global unicast IPv6 address
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *) ifa->ifa_addr;
            if (isGlobalUnicastIPv6(addr)) {
                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL,
                                0, NI_NUMERICHOST) == 0) {
                    LOGI("Global Unicast IPv6 Address: %s", host); // Log the IPv6 address
                    ipAddress = host;
                    break;
                }
            }
        } else if (family == AF_INET) {
            // Check for public IPv4 address
            struct sockaddr_in *addr = (struct sockaddr_in *) ifa->ifa_addr;
            if (isPublicIPv4(addr)) {
                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL,
                                0, NI_NUMERICHOST) == 0) {
                    LOGI("Public IPv4 Address: %s", host); // Log the public IPv4 address
                    ipAddress = host;
                    break;
                }
            } else if (isPrivateIPv4(addr)) {
                // Save any private IPv4 address as a fallback
                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL,
                                0, NI_NUMERICHOST) == 0) {
                    LOGI("Private IPv4 Address (Fallback): %s",host); // Log the private IPv4 address
                    fallbackIPv4 = host;
                }
            }
        }
    }

    // If no suitable IP address was found, use the fallback IPv4 address
    if (ipAddress == "Unable to get IP address" && !fallbackIPv4.empty()) {
        ipAddress = fallbackIPv4;
    }

    freeifaddrs(ifaddr);
    return env->NewStringUTF(ipAddress.c_str());
}