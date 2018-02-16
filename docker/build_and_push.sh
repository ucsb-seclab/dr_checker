#!/bin/bash
docker-compose -p drchecker up -d --build
export DOCKER_CLIENT_TIMEOUT=150
export COMPOSE_HTTP_TIMEOUT=150
docker-compose push
