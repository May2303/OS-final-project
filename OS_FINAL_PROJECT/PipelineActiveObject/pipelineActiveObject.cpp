#include "pipelineActiveObject.hpp"

/**
 * Constructor: Initializes a Pipeline object.
 * Takes a list of functions to be executed by the worker threads.
 */
Pipeline::Pipeline(const std::vector<std::function<void(void*)>>& functions) : f_stop(false) {
    // Populate the workers vector with Worker structs
    for (const auto& func : functions) {
        // Creating a mutex and condition variable for each worker
        std::mutex* task_qMutex = new std::mutex();
        std::condition_variable* taskCondition = new std::condition_variable();
        // Add a new worker to the vector with the given function, empty task queue, mutex, and condition variable
        workers.push_back({nullptr, func, std::queue<void*>(), task_qMutex, taskCondition, nullptr});
    }
    // Set the next_task_q pointer for each worker, except the last one, to point to the next worker's task queue
    for (size_t i = 0; i < workers.size() - 1; ++i) {
        workers[i].next_task_q = &workers[i + 1].task_q;
    }
}

/**
 * Destructor: Cleans up resources by stopping the worker threads and freeing memory.
 */
Pipeline::~Pipeline() {
    stop();  // Stop all worker threads

    // Iterate over all workers and free associated resources (threads, mutexes, condition variables)
    for (auto& worker : workers) {
        if (worker.thread && worker.thread->joinable()) {
            worker.thread->join();  // Ensure the thread has finished executing
        }
        // Free allocated memory for the thread, mutex, and condition variable
        delete worker.thread;
        delete worker.queueMutex;
        delete worker.condition;
    }
}

/**
 * Add a new task to the first worker's task queue.
 * This method locks the queue and notifies the first worker to start processing.
 */
void Pipeline::addTask(void* newTask) {
    std::lock_guard<std::mutex> lock(*(workers[0].queueMutex));
    workers[0].task_q.push(newTask);
    workers[0].condition->notify_one();  // Notify the first worker to start working on the new task
}

/**
 * Start all worker threads.
 * Iterates over all workers and creates threads for each, passing the workerFunction.
 */
void Pipeline::start() {
    f_stop = false;  // Reset the stop flag
    // Iterate over all workers and start a thread for each one
    for (size_t i = 0; i < workers.size(); ++i) {
        Worker* next_worker = (i + 1 < workers.size()) ? &workers[i + 1] : nullptr;  // If it's not the last worker, set the next worker
        workers[i].thread = new std::thread(&Pipeline::workerFunction, this, std::ref(workers[i]), next_worker);
    }
}

/**
 * Stop all worker threads.
 * Sets the stop flag to true and notifies all workers to stop processing.
 */
void Pipeline::stop() {
    f_stop = true;  // Signal the stop condition to all workers
    // Notify all workers to stop by unlocking their mutex and signaling their condition variables
    for (auto& worker : workers) {
        std::lock_guard<std::mutex> lock(*worker.queueMutex);
        worker.condition->notify_all();  // Notify all workers to exit
    }
}

/**
 * The worker function that continuously processes tasks until the stop flag is set.
 * Executes the assigned function on each task and forwards tasks to the next worker.
 */
void Pipeline::workerFunction(Worker& curr_worker, Worker* next_worker) {
    while (!f_stop) {
        // Task processing:
        void* task = nullptr;
        {
            std::unique_lock<std::mutex> lock(*curr_worker.queueMutex);
            // Wait until there's a task or the stop flag is set
            curr_worker.condition->wait(lock, [&]() { return f_stop || !curr_worker.task_q.empty(); });
            if (f_stop && curr_worker.task_q.empty()) return;  // Exit if stop flag is set and no tasks remain
            task = curr_worker.task_q.front();  // Get the task from the queue
            curr_worker.task_q.pop();  // Remove the task from the queue
        }
        // Execute the worker's function with the task, if available
        if (curr_worker.function) {
            curr_worker.function(task);
        }
        // If there is a next worker, forward the task to their queue
        if (next_worker) {
            std::lock_guard<std::mutex> lock(*next_worker->queueMutex);
            next_worker->task_q.push(task);  // Forward the task to the next worker
            next_worker->condition->notify_one();  // Notify the next worker to start working
        }
        if (f_stop) return;  // Check if stop flag was set mid-processing
    }
}
