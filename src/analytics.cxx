#include "analytics.hxx"
#include "exceptions.hxx"
#include "result.hxx"
#include <couchbase/analytics_scan_consistency.hxx>

couchbase::analytics_scan_consistency
str_to_scan_consistency_type(std::string consistency)
{
    if (consistency.compare("not_bounded") == 0) {
        return couchbase::analytics_scan_consistency::not_bounded;
    }
    if (consistency.compare("request_plus") == 0) {
        return couchbase::analytics_scan_consistency::request_plus;
    }

    // TODO: better exception
    PyErr_SetString(PyExc_ValueError, "Invalid Analytics Scan Consistency type.");
    return {};
}

PyObject*
get_result_metrics(couchbase::operations::analytics_response::analytics_metrics metrics)
{
    PyObject* pyObj_metrics = PyDict_New();
    std::chrono::duration<unsigned long long, std::nano> int_nsec = metrics.elapsed_time;
    PyObject* pyObj_tmp = PyLong_FromUnsignedLongLong(int_nsec.count());
    if (-1 == PyDict_SetItemString(pyObj_metrics, "elapsed_time", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    int_nsec = metrics.execution_time;
    pyObj_tmp = PyLong_FromUnsignedLongLong(int_nsec.count());
    if (-1 == PyDict_SetItemString(pyObj_metrics, "execution_time", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyLong_FromUnsignedLongLong(metrics.result_count);
    if (-1 == PyDict_SetItemString(pyObj_metrics, "result_count", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyLong_FromUnsignedLongLong(metrics.result_size);
    if (-1 == PyDict_SetItemString(pyObj_metrics, "result_size", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyLong_FromUnsignedLongLong(metrics.error_count);
    if (-1 == PyDict_SetItemString(pyObj_metrics, "error_count", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyLong_FromUnsignedLongLong(metrics.processed_objects);
    if (-1 == PyDict_SetItemString(pyObj_metrics, "processed_objects", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyLong_FromUnsignedLongLong(metrics.warning_count);
    if (-1 == PyDict_SetItemString(pyObj_metrics, "warning_count", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    return pyObj_metrics;
}

PyObject*
get_result_metadata(couchbase::operations::analytics_response::analytics_meta_data metadata, bool include_metrics)
{
    PyObject* pyObj_metadata = PyDict_New();
    PyObject* pyObj_tmp = PyUnicode_FromString(metadata.request_id.c_str());
    if (-1 == PyDict_SetItemString(pyObj_metadata, "request_id", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyUnicode_FromString(metadata.client_context_id.c_str());
    if (-1 == PyDict_SetItemString(pyObj_metadata, "client_context_id", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    pyObj_tmp = PyUnicode_FromString(metadata.status.c_str());
    if (-1 == PyDict_SetItemString(pyObj_metadata, "status", pyObj_tmp)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_tmp);

    if (metadata.signature.has_value()) {
        pyObj_tmp = json_decode(metadata.signature.value().c_str(), metadata.signature.value().length());
        if (-1 == PyDict_SetItemString(pyObj_metadata, "signature", pyObj_tmp)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_tmp);
    }

    PyObject* pyObj_warnings = PyList_New(static_cast<Py_ssize_t>(0));
    for (auto const& warning : metadata.warnings) {
        PyObject* pyObj_warning = PyDict_New();

        pyObj_tmp = PyLong_FromLong(warning.code);
        if (-1 == PyDict_SetItemString(pyObj_warning, "code", pyObj_tmp)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_tmp);

        pyObj_tmp = PyUnicode_FromString(warning.message.c_str());
        if (-1 == PyDict_SetItemString(pyObj_warning, "message", pyObj_tmp)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_tmp);

        if (-1 == PyList_Append(pyObj_warnings, pyObj_warning)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_warning);
    }

    if (-1 == PyDict_SetItemString(pyObj_metadata, "warnings", pyObj_warnings)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_warnings);

    PyObject* pyObj_errors = PyList_New(static_cast<Py_ssize_t>(0));
    for (auto const& error : metadata.errors) {
        PyObject* pyObj_error = PyDict_New();

        pyObj_tmp = PyLong_FromLong(error.code);
        if (-1 == PyDict_SetItemString(pyObj_error, "code", pyObj_tmp)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_tmp);

        pyObj_tmp = PyUnicode_FromString(error.message.c_str());
        if (-1 == PyDict_SetItemString(pyObj_error, "message", pyObj_tmp)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_tmp);

        if (-1 == PyList_Append(pyObj_errors, pyObj_error)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObj_error);
    }

    if (-1 == PyDict_SetItemString(pyObj_metadata, "errors", pyObj_errors)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_errors);

    if (include_metrics) {
        PyObject* pyObject_metrics = get_result_metrics(metadata.metrics);

        if (-1 == PyDict_SetItemString(pyObj_metadata, "metrics", pyObject_metrics)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_XDECREF(pyObject_metrics);
    }

    return pyObj_metadata;
}

result*
create_result_from_analytics_response(couchbase::operations::analytics_response resp, bool include_metrics)
{
    PyObject* pyObj_result = create_result_obj();
    result* res = reinterpret_cast<result*>(pyObj_result);
    res->ec = resp.ctx.ec;

    PyObject* pyObj_payload = PyDict_New();

    PyObject* pyObject_metadata = get_result_metadata(resp.meta, include_metrics);
    if (-1 == PyDict_SetItemString(pyObj_payload, "metadata", pyObject_metadata)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObject_metadata);

    if (-1 == PyDict_SetItemString(res->dict, RESULT_VALUE, pyObj_payload)) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_XDECREF(pyObj_payload);

    return res;
}

// void
// create_analytics_result(couchbase::operations::analytics_response resp, bool include_metrics, struct callback_context ctx)
// {
//     PyGILState_STATE state = PyGILState_Ensure();
//     PyObject* pyObj_args = NULL;
//     PyObject* pyObj_func = NULL;

//     if (resp.ctx.ec.value()) {
//         PyObject* pyObj_exc = build_exception(resp.ctx);
//         pyObj_func = ctx.get_errback();
//         pyObj_args = PyTuple_New(1);
//         PyTuple_SET_ITEM(pyObj_args, 0, pyObj_exc);
//     } else {
//         auto res = create_result_from_analytics_response(resp, include_metrics);
//         // TODO:  check if PyErr_Occurred() != nullptr and raise error accordingly
//         pyObj_func = ctx.get_callback();
//         pyObj_args = PyTuple_New(1);
//         PyTuple_SET_ITEM(pyObj_args, 0, reinterpret_cast<PyObject*>(res));
//     }

//     PyObject* pyObj_callback_res = PyObject_CallObject(const_cast<PyObject*>(pyObj_func), pyObj_args);
//     if (pyObj_callback_res) {
//         Py_XDECREF(pyObj_callback_res);
//     } else {
//         PyErr_Print();
//     }

//     Py_XDECREF(pyObj_args);
//     Py_XDECREF(pyObj_func);
//     ctx.decrement_PyObjects();
//     PyGILState_Release(state);
// }

void
create_analytics_result(couchbase::operations::analytics_response resp,
                        bool include_metrics,
                        std::shared_ptr<rows_queue<PyObject*>> rows,
                        PyObject* pyObj_callback,
                        PyObject* pyObj_errback)
{
    auto set_exception = false;

    PyGILState_STATE state = PyGILState_Ensure();
    Py_INCREF(Py_None);
    rows->put(Py_None);

    if (resp.ctx.ec.value()) {
        PyObject* pyObj_result = create_result_obj();
        result* res = reinterpret_cast<result*>(pyObj_result);
        res->ec = resp.ctx.ec;

        PyObject* pyObj_exc = build_exception_from_context(resp.ctx);
        if (-1 == PyDict_SetItemString(res->dict, "exc", pyObj_exc)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_DECREF(pyObj_exc);

        PyObject* pyObj_exc_details = pycbc_get_exception_kwargs("Error doing analytics operation.", __FILE__, __LINE__);
        if (-1 == PyDict_SetItemString(res->dict, "exc_details", pyObj_exc_details)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_DECREF(pyObj_exc_details);

        if (-1 == PyDict_SetItemString(res->dict, "has_exception", Py_True)) {
            PyErr_Print();
            PyErr_Clear();
        }

        // lets clear any errors
        PyErr_Clear();
        rows->put(reinterpret_cast<PyObject*>(res));
    } else {
        auto res = create_result_from_analytics_response(resp, include_metrics);

        if (res == nullptr || PyErr_Occurred() != nullptr) {
            set_exception = true;
        } else {
            rows->put(reinterpret_cast<PyObject*>(res));
        }
    }

    if (set_exception) {
        PyObject* pyObj_result = create_result_obj();
        result* res = reinterpret_cast<result*>(pyObj_result);

        PyObject* pyObj_exc_details =
          pycbc_core_get_exception_kwargs("Analytics operation error.", PycbcError::UnableToBuildResult, __FILE__, __LINE__);
        if (-1 == PyDict_SetItemString(res->dict, "exc_details", pyObj_exc_details)) {
            PyErr_Print();
            PyErr_Clear();
        }
        Py_DECREF(pyObj_exc_details);

        if (-1 == PyDict_SetItemString(res->dict, "has_exception", Py_True)) {
            PyErr_Print();
            PyErr_Clear();
        }
        rows->put(reinterpret_cast<PyObject*>(res));
    }
    Py_XDECREF(pyObj_errback);
    Py_XDECREF(pyObj_callback);
    PyGILState_Release(state);
}

streamed_result*
handle_analytics_query([[maybe_unused]] PyObject* self, PyObject* args, PyObject* kwargs)
{
    // need these for all operations
    PyObject* pyObj_conn = nullptr;
    char* statement = nullptr;

    char* scan_consistency = nullptr;
    char* bucket_name = nullptr;
    char* scope_name = nullptr;
    char* scope_qualifier = nullptr;
    char* client_context_id = nullptr;

    uint64_t timeout = 0;
    // booleans, but use int to read from kwargs
    int metrics = 0;
    int readonly = 0;
    int priority = 0;

    PyObject* pyObj_raw = nullptr;
    PyObject* pyObj_named_parameters = nullptr;
    PyObject* pyObj_positional_parameters = nullptr;
    PyObject* pyObj_serializer = nullptr;
    PyObject* pyObj_callback = nullptr;
    PyObject* pyObj_errback = nullptr;
    PyObject* pyObj_row_callback = nullptr;

    static const char* kw_list[] = { "conn",
                                     "statement",
                                     "bucket_name",
                                     "scope_name",
                                     "scope_qualifier",
                                     "client_context_id",
                                     "scan_consistency",
                                     "timeout",
                                     "metrics",
                                     "readonly",
                                     "priority",
                                     "named_parameters",
                                     "positional_parameters",
                                     "raw",
                                     "serializer",
                                     "callback",
                                     "errback",
                                     "row_callback",
                                     nullptr };

    const char* kw_format = "O!s|sssssLiiiOOOOOOO";
    int ret = PyArg_ParseTupleAndKeywords(args,
                                          kwargs,
                                          kw_format,
                                          const_cast<char**>(kw_list),
                                          &PyCapsule_Type,
                                          &pyObj_conn,
                                          &statement,
                                          &bucket_name,
                                          &scope_name,
                                          &scope_qualifier,
                                          &client_context_id,
                                          &scan_consistency,
                                          &timeout,
                                          &metrics,
                                          &readonly,
                                          &priority,
                                          &pyObj_named_parameters,
                                          &pyObj_positional_parameters,
                                          &pyObj_raw,
                                          &pyObj_serializer,
                                          &pyObj_callback,
                                          &pyObj_errback,
                                          &pyObj_row_callback);
    if (!ret) {
        PyErr_SetString(PyExc_ValueError, "Unable to parse arguments");
        return nullptr;
    }

    connection* conn = nullptr;
    std::chrono::milliseconds timeout_ms = couchbase::timeout_defaults::query_timeout;

    conn = reinterpret_cast<connection*>(PyCapsule_GetPointer(pyObj_conn, "conn_"));
    if (nullptr == conn) {
        PyErr_SetString(PyExc_ValueError, "passed null connection");
        return nullptr;
    }
    PyErr_Clear();

    if (0 < timeout) {
        timeout_ms = std::chrono::milliseconds(std::max(0ULL, timeout / 1000ULL));
    }

    couchbase::operations::analytics_request req{ statement };
    // positional parameters
    std::vector<couchbase::json_string> positional_parameters{};
    if (pyObj_positional_parameters && PyList_Check(pyObj_positional_parameters)) {
        size_t nargs = static_cast<size_t>(PyList_Size(pyObj_positional_parameters));
        size_t ii;
        for (ii = 0; ii < nargs; ++ii) {
            PyObject* pyOb_param = PyList_GetItem(pyObj_positional_parameters, ii);
            if (!pyOb_param) {
                // TODO:  handle this better
                PyErr_SetString(PyExc_ValueError, "Unable to parse positional argument.");
                return nullptr;
            }
            // PyList_GetItem returns borrowed ref, inc while using, decr after done
            Py_INCREF(pyOb_param);
            if (PyUnicode_Check(pyOb_param)) {
                auto res = std::string(PyUnicode_AsUTF8(pyOb_param));
                positional_parameters.push_back(couchbase::json_string{ std::move(res) });
            }
            //@TODO: exception if this check fails??
            Py_DECREF(pyOb_param);
            pyOb_param = nullptr;
        }
    }
    if (positional_parameters.size() > 0) {
        req.positional_parameters = positional_parameters;
    }

    // named parameters
    std::map<std::string, couchbase::json_string> named_parameters{};
    if (pyObj_named_parameters && PyDict_Check(pyObj_named_parameters)) {
        PyObject *pyObj_key, *pyObj_value;
        Py_ssize_t pos = 0;

        // PyObj_key and pyObj_value are borrowed references
        while (PyDict_Next(pyObj_named_parameters, &pos, &pyObj_key, &pyObj_value)) {
            std::string k;
            if (PyUnicode_Check(pyObj_key)) {
                k = std::string(PyUnicode_AsUTF8(pyObj_key));
            }
            if (PyUnicode_Check(pyObj_value) && !k.empty()) {
                auto res = std::string(PyUnicode_AsUTF8(pyObj_value));
                named_parameters.emplace(k, couchbase::json_string{ std::move(res) });
            }
        }
    }
    if (named_parameters.size() > 0) {
        req.named_parameters = named_parameters;
    }

    req.timeout = timeout_ms;
    // req.metrics = metrics == 1;
    req.readonly = readonly == 1;
    req.priority = priority == 1;

    if (scan_consistency) {
        req.scan_consistency =
          str_to_scan_consistency_type<couchbase::analytics_scan_consistency>(scan_consistency);
    }

    // raw options
    std::map<std::string, couchbase::json_string> raw_options{};
    if (pyObj_raw && PyDict_Check(pyObj_raw)) {
        PyObject *pyObj_key, *pyObj_value;
        Py_ssize_t pos = 0;

        // PyObj_key and pyObj_value are borrowed references
        while (PyDict_Next(pyObj_raw, &pos, &pyObj_key, &pyObj_value)) {
            std::string k;
            if (PyUnicode_Check(pyObj_key)) {
                k = std::string(PyUnicode_AsUTF8(pyObj_key));
            }
            if (PyUnicode_Check(pyObj_value) && !k.empty()) {
                auto res = std::string(PyUnicode_AsUTF8(pyObj_value));
                raw_options.emplace(k, couchbase::json_string{ std::move(res) });
            }
        }
    }
    if (raw_options.size() > 0) {
        req.raw = raw_options;
    }

    // PyObjects that need to be around for the cxx client lambda
    // have their increment/decrement handled w/in the callback_context struct
    // struct callback_context callback_ctx = { pyObj_callback, pyObj_errback };
    Py_XINCREF(pyObj_errback);
    Py_XINCREF(pyObj_callback);

    streamed_result* streamed_res = create_streamed_result_obj();

    req.row_callback = [rows = streamed_res->rows](std::string&& row) {
        PyGILState_STATE state = PyGILState_Ensure();
        PyObject* pyObj_row = PyBytes_FromStringAndSize(row.c_str(), row.length());
        rows->put(pyObj_row);
        PyGILState_Release(state);
        return couchbase::utils::json::stream_control::next_row;
    };

    Py_BEGIN_ALLOW_THREADS conn->cluster_->execute(req,
                                                   [rows = streamed_res->rows, include_metrics = metrics, pyObj_callback, pyObj_errback](
                                                     couchbase::operations::analytics_response resp) {
                                                       create_analytics_result(resp, include_metrics, rows, pyObj_callback, pyObj_errback);
                                                   });
    Py_END_ALLOW_THREADS

      return streamed_res;
}
