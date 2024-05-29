import { StatusCode } from 'grpc-web';
import React from 'react';

import { DebtSimplifierClient } from 'proto/service_grpc_web_pb';
import { TestReq } from 'proto/service_pb';

export function App() {
  console.log(`starting rpc to http://${window.location.hostname}:3001`);
  const client = new DebtSimplifierClient(
    `http://${window.location.hostname}:3001`,
    null,
    null
  );
  const req = new TestReq();
  req.setMsg('test guy');
  client.test(req, undefined, (error, res) => {
    if (error.code !== StatusCode.OK) {
      console.log('error:', error);
    } else {
      console.log(`Got response : ${res.getMsg()}`);
    }
  });
  return <></>;
}
