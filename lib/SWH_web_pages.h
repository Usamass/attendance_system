char loginPage[] = 
"<!DOCTYPE html>\n"
"<html>\n"
"  <head>\n"
"    <title>Login Page</title>\n"
"    <style>\n"
"      /* Simple CSS for centering the form on the page */\n"
"      body {\n"
"        display: flex;\n"
"        justify-content: center;\n"
"        align-items: center;\n"
"        height: 100vh;\n"
"      }\n"
"\n"
"      form {\n"
"        width: 300px;\n"
"        padding: 20px;\n"
"        background-color: #f5f5f5;\n"
"        border-radius: 5px;\n"
"        box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);\n"
"      }\n"
"\n"
"      label {\n"
"        display: block;\n"
"        margin-bottom: 10px;\n"
"      }\n"
"\n"
"      input[type=\"text\"],\n"
"      input[type=\"password\"] {\n"
"        width: 100%;\n"
"        padding: 10px;\n"
"        border-radius: 3px;\n"
"        border: none;\n"
"        margin-bottom: 20px;\n"
"        box-sizing: border-box;\n"
"      }\n"
"\n"
"      button[type=\"submit\"] {\n"
"        background-color: #008cba;\n"
"        border: none;\n"
"        color: white;\n"
"        padding: 10px 20px;\n"
"        border-radius: 3px;\n"
"        cursor: pointer;\n"
"      }\n"
"\n"
"      button[type=\"submit\"]:hover {\n"
"        background-color: #006a8e;\n"
"      }\n"
"    </style>\n"
"  </head>\n"
"\n"
"  <body>\n"
"    <form onsubmit=\"return submitForm(event)\">\n"
"      <h1>Login</h1>\n"
"      <label>Username:</label>\n"
"      <input type=\"text\" id=\"username\" required>\n"
"\n"
"      <label>Password:</label>\n"
"      <input type=\"password\" id=\"password\" required>\n"
"\n"
"      <button type=\"submit\">Submit</button>\n"
"    </form>\n"
"\n"
"    <script>\n"
"      // Function to submit the login form\n"
"      function submitForm(event) {\n"
"        // Prevent the default form submission behavior\n"
"        event.preventDefault();\n"
"\n"
"        // Get the values from the form fields\n"
"        const username = document.getElementById(\"username\").value;\n"
"        const password = document.getElementById(\"password\").value;\n"
"\n"
"        // Send a POST request to the server with the login information\n"
"        fetch(\"http://192.168.8.109/getData\", {\n"
"          method: \"POST\",\n"
"          headers: {\n"
"            \"Content-Type\": \"application/json\",\n"
"          },\n"
"          body: JSON.stringify({ username, password }),\n"
"        })\n"
"          .then((response) => {\n"
"            // If the response status is ok, redirect to the dashboard\n"
"            if (response.ok) {\n"
"              window.location.href = \"http://192.168.8.109/dashboard\";\n"
"            } else {\n"
"              // Otherwise, display an error message\n"
"              alert(\"Invalid username or password\");\n"
"            }\n"
"          })\n"
"          .catch((error) => {\n"
"            console.error(error);\n"
"          });\n"
"      }\n"
"    </script>\n"
"  </body>\n"
"</html>\n"
;

char dashboard [] = 
"<!DOCTYPE html>\n"
"<html>\n"
"\n"
"<head>\n"
"  <title>Biometric Attendance System</title>\n"
"  <style>\n"
"    body {\n"
"      font-family: Arial, sans-serif;\n"
"      margin: 0;\n"
"      padding: 0;\n"
"    }\n"
"\n"
"    #header {\n"
"      background-color: #333;\n"
"      color: white;\n"
"      padding: 10px;\n"
"    }\n"
"\n"
"    #menu {\n"
"      background-color: #f2f2f2;\n"
"      height: 100%;\n"
"      padding: 10px;\n"
"      width: 200px;\n"
"      position: fixed;\n"
"      top: 101px;\n"
"      left: 0;\n"
"    }\n"
"\n"
"    #menu ul {\n"
"      list-style-type: none;\n"
"      margin: 0;\n"
"      padding: 0;\n"
"    }\n"
"\n"
"    #menu li {\n"
"      padding: 10px;\n"
"      border-bottom: 1px solid #ccc;\n"
"      cursor: pointer;\n"
"    }\n"
"\n"
"    #menu li:hover {\n"
"      background-color: #ddd;\n"
"    }\n"
"\n"
"    #data {\n"
"      margin-left: 220px;\n"
"      margin-top: 80px;\n"
"      padding: 10px;\n"
"    }\n"
"  </style>\n"
"</head>\n"
"\n"
"<body>\n"
"  <div id=\"header\">\n"
"    <h1>SWH-Biometric Attendance System</h1>\n"
"  </div>\n"
"  <div id=\"menu\">\n"
"    <ul>\n"
"      <li onclick=\"getData('network')\">Network</li>\n"
"      <li onclick=\"getData('device')\">Device Configurations</li>\n"
"      <li onclick=\"getData('enrollment')\">Enrollment</li>\n"
"      <li onclick=\"getData('datetime')\">Date and Time</li>\n"
"      <li onclick=\"getData('logout')\">Logout</li>\n"
"    </ul>\n"
"  </div>\n"
"  <div id=\"data\"></div>\n"
"\n"
"  <script>\n"
"    function getData(setting) {\n"
"      const url = `http://192.168.8.109/${setting}`;\n"
"      const login_redirect = `http://192.168.8.109/login`;\n"
"\n"
"      if (setting === \"device\") {\n"
"        const formHtml = `\n"
"        <form style=\"display: flex; flex-direction: column; align-items: center; margin-top: 50px;\">\n"
"  <div style=\"display: flex; flex-direction: column; align-items: center; margin-bottom: 10px;\">\n"
"    <label for=\"device_id\" style=\"margin-right: 10px;\">Device ID:</label>\n"
"    <input type=\"text\" id=\"device_id\" name=\"device_id\" required style=\"padding: 8px; border: none; border-bottom: 2px solid #ddd; width: 100%; margin-bottom: 10px;\">\n"
"  </div>\n"
"  <div style=\"display: flex; flex-direction: column; align-items: center; margin-bottom: 10px;\">\n"
"    <label for=\"auth_token\" style=\"margin-right: 10px;\">Auth Token:</label>\n"
"    <input type=\"text\" id=\"auth_token\" name=\"auth_token\" required style=\"padding: 8px; border: none; border-bottom: 2px solid #ddd; width: 100%; margin-bottom: 10px;\">\n"
"  </div>\n"
"  <div style=\"display: flex; flex-direction: column; align-items: center; margin-bottom: 10px;\">\n"
"    <label for=\"device_location\" style=\"margin-right: 10px;\">Device Location:</label>\n"
"    <input type=\"text\" id=\"device_location\" name=\"device_location\" required style=\"padding: 8px; border: none; border-bottom: 2px solid #ddd; width: 100%; margin-bottom: 10px;\">\n"
"  </div>\n"
"  <div style=\"display: flex; flex-direction: column; align-items: center; margin-bottom: 10px;\">\n"
"    <label for=\"device_id\" style=\"margin-right: 10px;\">Device ID:</label>\n"
"    <input type=\"text\" id=\"device_id\" name=\"device_id\" required style=\"padding: 8px; border: none; border-bottom: 2px solid #ddd; width: 100%; margin-bottom: 10px;\">\n"
"  </div>\n"
"  <button type=\"submit\" style=\"padding: 10px; background-color: #333; color: white; border: none; border-radius: 5px; cursor: pointer;\">Save</button>\n"
"</form>\n"
"`;\n"
"\n"
"        document.getElementById('data').innerHTML = formHtml;\n"
"      }\n"
"      else if (setting == \"datetime\") {\n"
"        const formHtml = `\n"
"        <form id=\"datetime-form\" style=\"margin: auto; width: 50%; padding: 20px; border: 1px solid #ddd; border-radius: 5px;\">\n"
"        <label for=\"date\">Date:</label>\n"
"        <input type=\"date\" id=\"date\" name=\"date\" style=\"margin-bottom: 10px; padding: 5px; border-radius: 5px; border: 1px solid #ccc;\"><br>\n"
"        <label for=\"time\">Time:</label>\n"
"        <input type=\"time\" id=\"time\" name=\"time\" style=\"margin-bottom: 10px; padding: 5px; border-radius: 5px; border: 1px solid #ccc;\"><br>\n"
"        <button type=\"submit\" style=\"padding: 10px; background-color: #333; color: white; border: none; border-radius: 5px; cursor: pointer;\">Save</button>\n"
"      </form>\n"
"            `;\n"
"          \n"
"          document.getElementById('data').innerHTML = formHtml;\n"
"          document.getElementById('datetime-form').addEventListener('submit', submitDatetimeForm);\n"
"      }\n"
"      else if (setting == \"logout\") {\n"
"        fetch(\"http://192.168.8.109/logout\", {\n"
"          method: \"GET\",\n"
"          headers: {\n"
"            \"Content-Type\": \"application/json\",\n"
"          }\n"
"        }).then((response) => {\n"
"          // If the response status is ok, redirect to the dashboard\n"
"          if (response.ok) {\n"
"            window.location.href = login_redirect;\n"
"\n"
"\n"
"          }\n"
"        })\n"
"\n"
"\n"
"      }\n"
"      else if (setting == \"network\") {\n"
"const formHtml = `<form id=\"my-form\" style=\"display: flex; flex-direction: column; align-items: center;\">\n"
"  <label for=\"ip\" style=\"margin-bottom: 10px;\">IP Address:</label>\n"
"  <input type=\"text\" id=\"ip\" name=\"ip\" readonly style=\"padding: 5px; border: none; border-radius: 5px; background-color: #f2f2f2; margin-bottom: 10px; text-align: center\"><br>\n"
"  <label for=\"mac\" style=\"margin-bottom: 10px;\">MAC Address:</label>\n"
"  <input type=\"text\" id=\"mac\" name=\"mac\" readonly style=\"padding: 5px; border: none; border-radius: 5px; background-color: #f2f2f2; margin-bottom: 10px; text-align: center\"><br>\n"
"</form>\n"
"`;"
"document.getElementById('data').innerHTML = formHtml;\n"
"getNetworkData();\n"
"\n"
"\n"
"\n"
"      }\n"
"\n"
"      else {\n"
"        fetch(url)\n"
"          .then(response => response.text())\n"
"          .then(data => {\n"
"            document.getElementById('data').textContent = data;\n"
"          })\n"
"          .catch(error => console.error(error));\n"
"      }\n"
"    }\n"
"\n"
"    function submitDatetimeForm(e){\n"
"      e.preventDefault();\n"
"      const date = document.getElementById('date').value;\n"
"      const time = document.getElementById('time').value;\n"
"\n"
"      const datetime = `${date} ${time}`;\n"
"      const url = 'http://192.168.8.109/datetime';\n"
"\n"
"      fetch(url, {\n"
"        method: 'POST',\n"
"        body: JSON.stringify({date: date , time: time}),\n"
"        headers: {\n"
"          'Content-Type': 'application/json'\n"
"        }\n"
"      })\n"
"    }\n"
"\n"
"  function getNetworkData(){\n"
"    fetch('http://192.168.8.109/network')\n"
"    .then(response => response.json())\n"
"    .then(data => {\n"
"      document.getElementById('ip').value = data.ip;\n"
"      document.getElementById('mac').value = data.mac;\n"
"    });\n"
"  }\n"
"  </script>\n"
"</body>\n"
"\n"
"</html>"
;