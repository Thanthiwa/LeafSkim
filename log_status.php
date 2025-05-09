<?php
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "iot";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("❌ Connection failed: " . $conn->connect_error);
}

$switch = $_GET['switch']; 
$ssr = $_GET['ssr'];
$cost = isset($_GET['cost']) ? $_GET['cost'] : 0;

$sql = "INSERT INTO status (switch_state, ssr_state, cost) VALUES ('$switch', '$ssr', '$cost')";

if ($conn->query($sql) === TRUE) {
    echo "✔️ บันทึกสำเร็จ";
} else {
    echo "❌ Error: " . $conn->error;
}
$conn->close();
?>
