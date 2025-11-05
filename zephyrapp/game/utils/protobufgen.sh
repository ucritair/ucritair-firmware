cd src/comms/protobufs
python ${NANOPB_DIR}/generator/nanopb_generator.py -I${PROTOBUFS_DIR} ${PROTOBUFS_DIR}/**/*.proto