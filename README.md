[![CircleCI](https://circleci.com/gh/uofuseismo/umps.svg?style=svg&circle-token=build_status)](https://circleci.com/gh/uofuseismo/umps)

# About 

The project documentation can be found [here](https://uofuseismo.github.io/umps).

Utah Message Passing System is a network-seismology-oriented message passing library that

   1. Allows for multiple communication patterns - e.g., pub/sub, request/reply between different computers on a network.
   2. Addresses the network exploration problem with a hub and spoke model.
   3. Can validate connections.

This software was initially presented in On-premises Integration of Machine Learning Models at UUSS - Distributed Computing and Messaging at the SSA 2022 conference and Using the University of Utah Message Passing System to Help Realize Real-Time Machine Learning Modules in Network Operations at the SSA 2023 conference.

# Status

The software is currently under active development.  An initial release is slated for mid-2023.  The major development pushes right now are:

   1. Improving remote monitoring of and interaction with modules.
   2. Creating a doxygen-based manual.
   3. Migrating some modules to a subsequent library - [Utah Real-Time Seismology](https://github.com/uofuseismo/urts).
   4. Continuous delivery.

Expect the API to change without notice.

# Compiling

Check this [link](https://uofuseismo.github.io/umps/_topic_install.html).
