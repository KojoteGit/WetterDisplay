<?php

    $SEPARATOR = '#';
    $app_id = '<APP_ID>';
    $app_secret = '<APP_SECRET>';
    $username = '<USER_NAME>';
    $password = '<USER_PASSWORD>';
    $scope = 'read_station'; // all scopes are selected.
    $token_url = "https://api.netatmo.com/oauth2/token";
    $postdata = http_build_query(
        array(
            'grant_type' => "password",
            'client_id' => $app_id,
            'client_secret' => $app_secret,
            'username' => $username,
            'password' => $password,
            'scope' => $scope
        )
    );

    $opts = array('http' =>
    array(
        'method'  => 'POST',
        'header'  => 'Content-type: application/x-www-form-urlencoded',
        'content' => $postdata
    ),
    );

    $context  = stream_context_create($opts);

    $response = file_get_contents($token_url, false, $context);

    $params = null;
    $params = json_decode($response, true);

    //nachdem die Anmeldung funktioniert hat, werden ein kompletter Datensatz eingelesen
    $api_url = "https://api.netatmo.com/api/getstationsdata?access_token="
    . $params['access_token'];

    $dataResponse = json_decode(file_get_contents($api_url),true);

    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["dashboard_data"]["Temperature"];
    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["modules"][0]["dashboard_data"]["Temperature"];
    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["dashboard_data"]["CO2"];
    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["dashboard_data"]["Humidity"];
    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["modules"][0]["dashboard_data"]["Humidity"];
    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["modules"][1]["dashboard_data"]["sum_rain_1"];
    echo $SEPARATOR;
    echo $dataResponse["body"]["devices"][0]["modules"][1]["dashboard_data"]["sum_rain_24"];
    echo $SEPARATOR;

    ?>
