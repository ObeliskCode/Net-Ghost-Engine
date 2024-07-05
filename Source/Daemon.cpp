#include "Daemon.h"

Daemon* Daemon::instance = nullptr; // definition class variable

// rules: multi-threaded work is always batched IO & multi-threaded work is mostly sequential anyway.

Daemon::Daemon() {
    const auto workerCt = m_processor_count - 1;
    std::cout << "workerCt " << workerCt << std::endl;

    if (workerCt <= 0) {
        single_threaded = true;
        return;
    }

    for (unsigned short i = 1; i <= SHORT_ID_MAX; i++) {
		availableIDs.push(i);
	}

    op_in.push_back(OP_IN());
    op_out.push_back(OP_OUT());

    unsigned int poolCt = workerCt - 1;

    for (unsigned int i = 0; i < poolCt; i++) {
        op_in.push_back(OP_IN());
        op_out.push_back(OP_OUT());
        Workers.push_back(std::thread(&Daemon::pollWorker, this, i));
    }
    daemon = std::thread(&Daemon::pollDaemon, this);

}

Daemon::~Daemon() {
    threadStopped = true;
    daemon.join();
    for (int i = 0; i < Workers.size(); i++) {
        Workers[i].join();
    }
}

void Daemon::pollDaemon() {
	while (!threadStopped) {
        data_in_mutex.lock();
        if (data_in.size() > 0){
            /* pop first element off stack*/
            const DATA_IN t = data_in[0];
            data_in.erase(data_in.begin());
            data_in_mutex.unlock();

            /* do dispatch */
            OpFunc OF = std::get<0>(t);
            void* data = std::get<1>(t);

            for (int i = 0; i < op_in.size(); i++){
                op_in_mutex[i].lock();
            }

            // dispatch function could be templated out!
            OF.Dispatch(data, OF, op_in,op_out);

            for (int i = 0; i < op_in.size(); i++){
                op_in_mutex[i].unlock();
            }

            /* do MT operations */
            // for parity reasons we lock op_in_mutex[0]
            op_in_mutex[0].lock();

                for (auto it = op_in[0].begin(); it != op_in[0].end(); it++){
                    const OP_TUPLE_IN t = *it;
                    op_in[0].erase(it);

                    OP operation = std::get<0>(t);
                    unsigned int idx = std::get<2>(t);
                    void* datum = std::get<3>(t);

                    void* result = operation(datum);

                    op_out_mutex[0].lock();
                    op_out[0].push_back(OP_TUPLE_OUT(std::get<1>(t), idx, result));
                    op_out_mutex[0].unlock();
                }

            op_in_mutex[0].unlock();

            /* Package Data to be receieved by recProcess */
            data_out_mutex.lock();
            void** dl;

            for (int i = 0; i < op_out.size(); i++){
                op_out_mutex[i].lock();
            }
            // TODO: aggregate data for package
            for (int i = 0; i < op_out.size(); i++){
                op_out_mutex[i].lock();
            }

            // TODO: change Package func API!
            void* package = OF.Package(dl,OF.dataCt);
            data_out.push_back(DATA_OUT(OF.PID, package));

            data_out_mutex.unlock();
        } else {
            data_in_mutex.unlock();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void Daemon::pollWorker(unsigned int workerCt) {
    unsigned int busIndex = workerCt + 1;
    while (!threadStopped) {
            op_in_mutex[busIndex].lock();

                for (auto it = op_in[busIndex].begin(); it != op_in[busIndex].end(); it++){
                    const OP_TUPLE_IN t = *it;
                    op_in[busIndex].erase(it);

                    OP operation = std::get<0>(t);
                    unsigned int idx = std::get<2>(t);
                    void* datum = std::get<3>(t);

                    void* result = operation(datum);

                    op_out_mutex[busIndex].lock();
                    op_out[busIndex].push_back(OP_TUPLE_OUT(std::get<1>(t), idx, result));
                    op_out_mutex[busIndex].unlock();
                }

            op_in_mutex[busIndex].unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void* Daemon::blockingProcess(OpFunc of, void* data){
    if (single_threaded) return nullptr;
    // push datafunc tuple to data_in
    if (availableIDs.empty()) std::terminate();
    of.PID = availableIDs.front();
    availableIDs.pop();
    data_in_mutex.lock();
    data_in.push_back(DATA_IN(of, data));
    data_in_mutex.unlock();

    // check for packaged results in data_out
    while (true) {
        // encapsulate with atomic bool check here..
        data_out_mutex.lock();
            for (auto it = data_out.begin(); it != data_out.end(); it++){
                DATA_OUT t = *it;
                if (of.PID == std::get<0>(t)){
                    data_out.erase(it);
                    data_out_mutex.unlock();
                    return std::get<1>(t);
                }
            }
        data_out_mutex.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

}
