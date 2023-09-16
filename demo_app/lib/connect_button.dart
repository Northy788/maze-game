import 'package:flutter/material.dart';

class connectButton extends StatelessWidget {
  const connectButton({
    super.key,
    required this.connecting,
  });

  final Function() connecting;

  @override
  Widget build(BuildContext context) {
    return FilledButton(
      onPressed: connecting,
      child: const Text("CONNECT"),
    );
  }
}
