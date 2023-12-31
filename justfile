oci_tool := `which podman > /dev/null && echo podman || echo docker`
oci_volume_args := if oci_tool == "podman" { ' --ignore' } else { '' }
oci_run_args := if oci_tool == "podman" { '--security-opt label=disable' } else { '' }
build_dir := 'cmake-build-default'
image_base := 'tcenv'

_default:
  @just --list

_prep_volumes:
    {{oci_tool}} volume create{{oci_volume_args}} tcenv-tcroot > /dev/null
    {{oci_tool}} volume create{{oci_volume_args}} tcenv-build > /dev/null
    {{oci_tool}} volume create{{oci_volume_args}} tcenv-ccache > /dev/null

_run IMAGE *ARGS:
    @echo $(tput bold)docker run '...' {{image_base}}:{{IMAGE}} {{ARGS}}$(tput sgr0)
    @{{oci_tool}} run --rm -it {{oci_run_args}} -v tcenv-ccache:/root/.cache -v tcenv-tcroot:/tcroot -v `pwd`:/proj -v tcenv-build:/proj/{{build_dir}} -w /proj {{image_base}}:{{IMAGE}} {{ARGS}}

_tidy_impl:
    @find src -regex '.*\.\(cpp\|hpp\|cppm\)' -print -exec clang-tidy -p {{build_dir}} {} \;

_format_impl:
    @find src -regex '.*\.\(cpp\|hpp\|cppm\)' -print -exec clang-format --dry-run {} \;

# build container image
build-image IMAGE='proj': _prep_volumes
    {{oci_tool}} build -f oci/Dockerfile.{{IMAGE}} -t {{image_base}}:{{IMAGE}} oci

# enter default shell of container
shell IMAGE='proj': ( _run IMAGE )

# inspect container image
dive IMAGE='proj': ( build-image IMAGE )
    dive {{image_base}}:{{IMAGE}}

# browse files in container
lf IMAGE='proj': ( _run IMAGE 'lf' )

# build project. ARGS are passed to cmake, i.e. --target XXX
build *ARGS: ( _run 'proj' 'cmake --build --preset=default' ARGS )

# configure project. ARGS are passed to cmake, i.e. --fresh
configure *ARGS: ( _run 'proj' 'cmake --preset=default' ARGS )

# rebuild project with fresh configure
rebuild *ARGS: ( configure '--fresh' ) build

# rebuild docker, clean cache and then build project
full-rebuild: ( build-image 'proj' ) ( _run 'proj' 'ccache --clear' ) ( _run 'proj' ('rm -r ' + build_dir + '/* || echo clean') ) rebuild

# run 'tcenv' in specified stage. Example: just run 0 test
run STAGE *ARGS: build ( _run ('stage' + STAGE) (build_dir + '/tcenv') ARGS )

# run 'clang-tidy' and print warnings
check-tidy: ( _run 'proj' 'just _tidy_impl' )

# run 'clang-format' and print warnings
check-format: ( _run 'proj' 'just _format_impl' )
