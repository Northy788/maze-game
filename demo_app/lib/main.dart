import 'package:demo_app/connect_button.dart';
import 'package:demo_app/select_map.dart';
import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_core/firebase_core.dart';
import 'firebase_options.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  runApp(MainApp());
}

class MainApp extends StatefulWidget {
  MainApp({super.key});

  final firebaseApp = Firebase.app();
  get rtdb => FirebaseDatabase.instanceFor(
      app: firebaseApp,
      databaseURL:
          "https://maze-game-demo-cbd12-default-rtdb.asia-southeast1.firebasedatabase.app/");

  @override
  State<MainApp> createState() => _MainAppState();
}

class _MainAppState extends State<MainApp> {
  void sendData() {
    DatabaseReference ref = FirebaseDatabase.instance.ref("MAZE-GAME-Device");
    ref.set({
      "status": "CONNECTED",
    });
  }

  void sendMap(int map) {
    DatabaseReference ref = FirebaseDatabase.instance.ref("MAZE-GAME-Device");
    ref.set({
      "status": "PLAY",
      "map": map,
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          leading: const Icon(Icons.circle),
          title: const Text("MAZE GAME DEMO"),
          backgroundColor: Colors.black87,
          shadowColor: Colors.grey,
        ),
        body: Container(
          decoration: const BoxDecoration(
            color: Colors.black54,
          ),
          child: Center(
            child: Column(
              children: [
                const SizedBox(
                  height: 20,
                ),
                selectMap(
                  ledState: Icons.map_outlined,
                  changeMap: () => sendMap(1),
                  mapName: "MAP 1",
                ),
                const SizedBox(
                  height: 20,
                ),
                selectMap(
                  ledState: Icons.map_outlined,
                  changeMap: () => sendMap(2),
                  mapName: "MAP 2",
                ),
                const SizedBox(
                  height: 20,
                ),
                selectMap(
                  ledState: Icons.map_outlined,
                  changeMap: () => sendMap(3),
                  mapName: "MAP 3",
                ),
                const SizedBox(
                  height: 20,
                ),
                selectMap(
                  ledState: Icons.map_outlined,
                  changeMap: () => sendMap(4),
                  mapName: "MAP 4",
                ),
                const SizedBox(
                  height: 20,
                ),
                selectMap(
                  ledState: Icons.map_outlined,
                  changeMap: () => sendMap(5),
                  mapName: "MAP 5",
                ),
                const SizedBox(
                  height: 30,
                ),
                connectButton(connecting: sendData),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
