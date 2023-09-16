import 'package:flutter/material.dart';

class selectMap extends StatelessWidget {
  const selectMap({
    super.key,
    required this.ledState,
    required this.changeMap,
    required this.mapName,
  });

  final IconData ledState;
  final Function() changeMap;
  final String mapName;
  @override
  Widget build(BuildContext context) {
    return TextButton.icon(
      onPressed: changeMap,
      icon: Icon(
        ledState,
        color: Colors.white,
        size: 80,
      ),
      label: Text(
        mapName,
        style: const TextStyle(
          color: Colors.grey,
        ),
      ),
      style: TextButton.styleFrom(
        backgroundColor: Colors.black,
        fixedSize: Size(200, 100),
        side: const BorderSide(
          color: Colors.black,
        ),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(15),
        ),
      ),
    );
  }
}
