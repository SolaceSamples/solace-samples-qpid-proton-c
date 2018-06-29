# Getting Started Samples

## Using Apache Qpid Proton C over AMQP 1.0 with Solace PubSub+ Message Brokers

The Advanced Message Queuing Protocol (AMQP) is an open standard application layer protocol for message-oriented middleware, and Solace PubSub+ Message Brokers support AMQP 1.0.

In addition to information provided on the Solace [Developer Portal](http://dev.solace.com/tech/amqp/), you may also look at external sources for more details about AMQP:

 - http://www.amqp.org
 - https://www.oasis-open.org/committees/tc_home.php?wg_abbrev=amqp
 - http://docs.oasis-open.org/amqp/core/v1.0/amqp-core-complete-v1.0.pdf

The "Getting Started" Samples will get you up to speed and sending messages with Solace technology as quickly as possible. There are three ways you can get started:

- Follow [these instructions](https://cloud.solace.com/create-messaging-service/) to quickly spin up a cloud-based Solace messaging service for your applications.
- Follow [these instructions](https://docs.solace.com/Solace-VMR-Set-Up/Setting-Up-VMRs.htm) to start the Solace PubSub+ software message broker in leading Clouds, Container Platforms or Hypervisors. The tutorials outline where to download and how to install the Solace PubSub+ software message broker.
- If your company has Solace PubSub+ appliances deployed, contact your middleware team to obtain the host name or IP address of a Solace PubSub+ appliance to test against, a username and password to access it, and a VPN in which you can produce and consume messages.

## Contents
This repository contains sample code for the following scenarios:

1. Publish to a Queue, see [send](src/send.c)
2. Receive from a Queue, see [receive](src/receive.c)
3. Publish on a Topic using address prefix, see [producer](src/producer.c)
4. Receive from Durable Topic Endpoint using address prefix, see [dte_solconsumer](src/dte_solconsumer.c)
5. Receive from Durable Topic Endpoint using address prefix and terminus durability fields, see [dte_consumer](src/dte_consumer.c)

>**Note** AMQP address prefixes are not supported until Solace PubSub+ software message broker **version 8.11.0** and Solace PubSub+ appliance **version 8.5.0**.

## Prerequisites

There are two environments for the 

### Docker Environment Prerequisites

Must have docker version 18 or later installed and available.
Must have bash shell script environment.

### Local Environment Prerequisites

Must have the Apache Qpid Proton C library  version 0.23 or later.
Must have make, and gcc tools available to use makefile.

##### Building Apache Qpid Proton locally
Check out the [images folder](modules/docker-qpid-proton/images) in the [docker-qpid-proton project](modules/docker-qpid-proton) for how the docker container built the Apache qpid Proton environment. For building on platforms not supported by the docker-qpid-proton project check out the [Apache Qpid Proton project](https://github.com/apache/qpid-proton).

## Building & Running with Docker

Building and Running in a Docker Container tailored to getting started with Apache Qpid Proton C quickly.

### Setup environment and Build the Examples with Docker
Just clone and make. For example:

  1. clone this GitHub repository
  2. `./env.sh make`

All executables are built using the docker container and place on the local host machine under `src/bin`.
Take a look at `./env.sh make help` for more details about make targets.

### Running the Examples with Docker

To try individual examples, build the project from source and then run them like the following:

    ./env.sh run send -a <msg_backbone_ip> -p <port>

>**Note:** the env.sh run <example_executable> for `send` does not have a path. All executables under `src/bin` can be run in the docker container using the `env.sh run` command. Look at `env.sh help` for details.

## Building and Running with Local Environment

### Build the Examples

Just clone and make. For example:

  1. clone this GitHub repository
  2. `make -f src/makefile`

All executables are built to the `src/bin` directory.

### Run the Examples

To try individual examples, build the project from source and then run them like the following:

    ./src/bin/send -a <msg_backbone_ip> -p <port>

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

See the list of [contributors](https://github.com/cjwomrgan-sol-sys/solace-samples-qpid-proton-c/graphs/contributors) who participated in this project.

## License

This project is licensed under the Apache License, Version 2.0. - See the [LICENSE](LICENSE) file for details.

## Resources

For more information try these resources:

- The Solace Developer Portal website at: http://dev.solace.com
- Get a better understanding of [Solace technology](http://dev.solace.com/tech/).
- Check out the [Solace blog](http://dev.solace.com/blog/) for other interesting discussions around Solace technology
- Ask the [Solace community.](http://dev.solace.com/community/)

