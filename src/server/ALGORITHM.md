//event string to pubsubevent (value is real element with memory) from eventstring to eventStructure
//this map store the static event
//the dynamic event store in the innermap of clienttoSub, the value represent the number of event published
//map<int, bool> map the required trigguring time of specific event into if it is satisfied
//for example, event1:1 represent the event1 needed to be published 1 time, the init value is false for this inner map
string is not the full format in this case
map<string, map<int, bool>> strtoEvent;

//client id to pubsubWrapper(value is real element with memory) from clientid to clientStructure
map<string, pubsubWrapper *> clientidtoWrapper;

// to pubsubWrapperid (value is pointer) from subeventstring to set of clientid
// the key here is full form insluding the publish number for example event1:1

map<string, set<string>> subtoClient;

// clientid to pubsubEvent (value is pointer) from clientid to map of subscribedEvent
// in the inner map, the integer represent the time that the event have been pushed
string is not the full format in this case
map<string, map<string, int>> clienttoSub;

1 traverse the event list

2 parse the event string, if there is no number the default required publish number is 1

3 map<int,bool> RequiredTriggureMap by strtoEvent according to global event list with the prefix of current event, such as event1:1 event1:2 event1:3 ...

4 get client set from subtoClient(client that associated with this event, index event here is in full format) and traverse the client set

5 clienttoSub[clientId][event]+1, newpublish=clienttoSub[clientId][event]

6 if RequirclienttoSub[clientId]edTriggureMap[newpublish] not exist, continue, else RequiredTriggureMap[newpublish]=true

7 set ifTrigure=true; check other relevent event, traverse map, clienttoSub[clientId] for every key, if RequirclienttoSub[key]==false, ifTrigure=fase, break;

8 if ifTrigure=true triggure and clean the clienttoSub[clientId]