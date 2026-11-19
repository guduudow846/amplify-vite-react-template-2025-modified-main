import { defineFunction } from '@aws-amplify/backend';

export const updateDeviceShadow = defineFunction({
  name: 'updateDeviceShadow',
  entry: './handler.ts',
});