#ifndef MULTI_MDNS_H
#define MULTI_MDNS_H

bool multi_mdns_query();
void multi_mdns_query_do();
void multi_mdns_query_close();

bool multi_mdns_service_init();
void multi_mdns_service_do();
void multi_mdns_service_close();

#endif
