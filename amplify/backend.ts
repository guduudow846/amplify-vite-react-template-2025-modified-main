import { defineBackend } from '@aws-amplify/backend';
import { auth } from './auth/resource';
import { data } from './data/resource';

import { graphqlIoTCoreTelemetry } from './functions/graphqlIoTCoreTelemetry/resource';
import { graphqlIoTCoreStatus } from './functions/graphqlIoTCoreStatus/resource';
import { pollSmhi } from './jobs/pollSmhi/resource';
import { updateDeviceShadow } from './functions/updateDeviceShadow/resource';
import { storeDeviceShadow } from './functions/storeDeviceShadow/resource';

import { Stack } from 'aws-cdk-lib';
import { PolicyStatement } from 'aws-cdk-lib/aws-iam';

const backend = defineBackend({
  auth,
  data,
  graphqlIoTCoreTelemetry,
  graphqlIoTCoreStatus,
  pollSmhi,
  updateDeviceShadow,
  storeDeviceShadow,
});

const stack = Stack.of(backend.updateDeviceShadow.resources.lambda);

backend.updateDeviceShadow.resources.lambda.addToRolePolicy(new PolicyStatement({
  actions: ['iot:DescribeEndpoint'],
  resources: ['*'],
}));

backend.updateDeviceShadow.resources.lambda.addToRolePolicy(new PolicyStatement({
  actions: ['iot:UpdateThingShadow'],
  resources: [`arn:aws:iot:${stack.region}:${stack.account}:thing/*`],
}));