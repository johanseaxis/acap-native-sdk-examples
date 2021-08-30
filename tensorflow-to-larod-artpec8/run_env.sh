# If the --no-gpu option is set, we should empty the GPU_FLAG to disable GPU
# acceleration in the container
GPU_FLAG="--gpus all"
for arg in "$@"; do
  shift
  case "$arg" in
    "--no-gpu") GPU_FLAG='' ;;
  esac
done

docker run $GPU_FLAG -v /var/run/docker.sock:/var/run/docker.sock --network host -v $(pwd)/env:/env -it tensorflow_to_larod-a8 /bin/bash
