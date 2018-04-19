#!/usr/bin/env bash

echo -n > output
taskset -c 3 ./resettimer
