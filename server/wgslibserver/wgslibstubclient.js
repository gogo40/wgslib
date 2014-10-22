function WGSLibStubClient(url) {
    this.url = url;
    var id = 1;
    
    function doJsonRpcRequest(method, params, methodCall, callback_success, callback_error) {
        var request = {};
        if (methodCall)
            request.id = id++;
        request.jsonrpc = "2.0";
        request.method = method;
        request.params = params
        JSON.stringify(request);
        
        $.ajax({
            type: "POST",
            url: url,
            data: JSON.stringify(request),
            success: function (response) {
                if (methodCall) {
                    if (response.hasOwnProperty("result") && response.hasOwnProperty("id")) {
                        callback_success(response.id, response.result);
                    } else if (response.hasOwnProperty("error")) {
                        if (callback_error != null)
                            callback_error(response.error.code,response.error.message);
                    } else {
                        if (callback_error != null)
                            callback_error(-32001, "Invalid Server response: " + response);
                    }
                }
            },
            error: function () {
                if (methodCall)
                    callback_error(-32002, "AJAX Error");
            },
            dataType: "json"
        });
        return id-1;
    }
    this.doRPC = function(method, params, methodCall, callback_success, callback_error) {
        return doJsonRpcRequest(method, params, methodCall, callback_success, callback_error);
    }
}

WGSLibStubClient.prototype.compute_variograms = function(X_prop, Y_prop, Z_prop, dimensions, directions, grid_name, num_lags, props, props_name, props_selected, callbackSuccess, callbackError) {
    var params = {X_prop : X_prop, Y_prop : Y_prop, Z_prop : Z_prop, dimensions : dimensions, directions : directions, grid_name : grid_name, num_lags : num_lags, props : props, props_name : props_name, props_selected : props_selected};
    return this.doRPC("compute_variograms", params, true, callbackSuccess, callbackError);
};
WGSLibStubClient.prototype.notifyServer = function(callbackSuccess, callbackError) {
    var params = null;
    this.doRPC("notifyServer", params, false, callbackSuccess, callbackError);
};
