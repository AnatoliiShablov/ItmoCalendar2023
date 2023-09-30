#!/bin/bash

set -euxo pipefail

function build() {
	echo "Building server"

	mkdir Server-build

	echo "Directory Server-build created"

	cmake -BServer-build -SServer -DCMAKE_BUILD_TYPE=Release
	cmake --build Server-build --parallel

	echo "Server builded successfully"

	echo "Building router"

	cp -r Router Router-build

	echo "Directory Router-build created"

	cd Router-build
	python3 -m venv venv

	echo "venv created"

	source venv/bin/activate
	pip install -r requirements.txt

	echo "Python deps installed"

	python -m grpc_tools.protoc -I ../API/ --python_out=. --pyi_out=. --grpc_python_out=. FrontAPI.proto

	echo "Python gRPC API generated"

	cd ..

	echo "Router builded successfully"

	echo "Build finished"
}

function run() {
	echo "Starting server..."
	$(cd Server-build && ./Server) &

	echo "Starting router..."
	$(cd Router-build && source venv/bin/activate && python ./main.py) &

	echo "Runing..."

	wait -n

	kill $(jobs -p)

	echo "Stopped"
}

function clean() {
	set +e
	echo "Removing server..."
	rm -r Server-build
	echo "Server removed"

	echo "Removing router..."
	rm -r Router-build
	echo "Router removed"

	echo "Clean finished"
	set -e
}

case "$1" in
"build")
	build
	;;

"run")
	run
	;;

"clean")
	clean
	;;

"*")
	echo "Run './build.sh build' or"
	echo "Run './build.sh run' or"
	echo "Run './build.sh clean'"
	;;
esac
