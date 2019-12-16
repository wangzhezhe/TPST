This project provides the message communication pattern based on the pub/sub interfaces. Another [project](https://github.com/wangzhezhe/MMServer) provides the message communication pattern based on the shared space and the message queue.

### Build 

The depedencies for this project is the grpc and protobuf. Refer to [this](https://github.com/IvanSafonov/grpc-cmake-example) about building grpc and protocol buffer. Then use the CMake to build the project.

### Main features

The pub/sub service is started by the MPI. Every MPI process run on one physical server, there is thread pool to manage the thread on specific server to utilize the multiple cpu if the cpu-per-task is larger than 1.

**pub-sub-notify**

The contents subscribed to the pub/sub server is supposed to be the message such as string. This string can be json string which contains the more complicated data structure. This string is supposed to be the metadata instead of the raw data (such as N-dimension array) of the simulation.

The main function of the pub/sub server is to execute the match function based on the subscribed and published event and execute the notify function when the match function returns true.

The main interface is the `publish` and `subscribe`. The `match function` and the `notify function` can be customized. 

When there is new published message comes, the pub/sub server will apply a thread from the thread pool to execute the match function. Only the messages that contain the same topic will be compared.

**dyanmic group management**

The group management is used for the scalability. When there is a subscribed message, the client will find a server according to the hash value of this message and send the subscribe request to this server. When there is few subscribers but lots of publishers, there is a large burden for a few servers even if there are multiple servers, and the resource can not be utilized fully. The group management divides all the servers into different groups, and the subscriber will send a request to the server located in a specific group. When there is a published message, it will broadcast this message to all the services within this group.

Besides, we also change the size of the group dynamically according to the workload (the number of the subscribed message) for a specific group. The group with a small number of the subscription will be merged into the group, which has a heavy burden. The threshold to trigger the group redistribution operation is calculated by average value. The group will not accept the request during the wall time (the subscribed message will be copied into the new group firstly, then the old message will be deleted, then the identity will be changed).

The information of the group for every server is maintained by the shared file on a parallel file system. It could also be maintained by every coordinator server in the future. (coordinator server is in charge of the communication between different groups)

**flexible matching mechanism**

The typical matching mechanism is to compare the literal value between the published topic and the subscribed topic. This is not flexible enough for scientific applications. We extract the comparison between the publish and subscribed information as a separate function. When this function returns true, the notified function can be triggered. For example, the matching function can be the comparison of the bounding box, and if there is overlapping, the notification will be triggered. The matching function can also be the threshold filter, and if the step is larger than a specific threshold, the notification will be triggered. The typical notification is to send the message from the publisher to the subscriber.
