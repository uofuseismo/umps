#ifndef EXAMPLES_XPUBXSUB_HPP
#define EXAMPLES_XPUBXSUB_HPP
#define N_PUBLISHERS 3
#define N_MESSAGES 4
void proxy();
void publisher(int publisherID = 1);
void subscriber(int subscriberID = 1);
#endif
